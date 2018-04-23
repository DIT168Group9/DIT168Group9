#include "V2VService.hpp"

std::shared_ptr<cluon::OD4Session> od4;

int main(int argc, char **argv) {
    int returnValue = 0;
    auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);
    if (0 == commandlineArguments.count("cid") || 0 == commandlineArguments.count("freq") ||
        0 == commandlineArguments.count("ip") || 0 == commandlineArguments.count("id")) {
        std::cerr << argv[0] << " sends and receives follower or leader status as defined in the V2V Protocol (DIT168)."
                  << std::endl;
        std::cerr << "Usage:   " << argv[0]
                  << " --cid=<OD4Session Session> --freq=<frequency> --ip=<localIP> --id=<DIT168Group>"
                  << std::endl;
        std::cerr << "Example: " << argv[0] << " --cid=111 --freq=10 --ip=127.0.0.1 --id=9" << std::endl;
        returnValue = 1;
    } else {
        const uint16_t CID = (uint16_t) std::stoi(commandlineArguments["cid"]);
        const uint16_t FREQ = (uint16_t) std::stoi(commandlineArguments["freq"]);
        const std::string IP = commandlineArguments["ip"];
        const std::string ID = commandlineArguments["id"];

        std::shared_ptr<V2VService> v2vService = std::make_shared<V2VService>(IP, ID);
        float pedalPos = 0, steeringAngle = 0;
        uint16_t button = 0;

        cluon::OD4Session od4(CID,
        [&v2vService, &pedalPos, &steeringAngle, &button](cluon::data::Envelope &&envelope) noexcept {
            if (envelope.dataType() == 1041) {
                opendlv::proxy::PedalPositionReading ppr =
                        cluon::extractMessage<opendlv::proxy::PedalPositionReading>(std::move(envelope));
                pedalPos = ppr.position();
            }
            else if (envelope.dataType() == 1043) {
                opendlv::proxy::ButtonPressed buttonPressed =
                        cluon::extractMessage<opendlv::proxy::ButtonPressed>(std::move(envelope));
                button = buttonPressed.buttonNumber();
                switch (button) {
                    case 3:
                        v2vService->announcePresence();
                        break;
                    default:
                        std::cout << "You shouldn't be here" << std::endl;
                }
            }
            else if (envelope.dataType() == 1045) {
                opendlv::proxy::GroundSteeringReading gsr =
                        cluon::extractMessage<opendlv::proxy::GroundSteeringReading>(std::move(envelope));
                steeringAngle = gsr.groundSteering();
            }
        });

        auto atFrequency{[&v2vService, &pedalPos, &steeringAngle]() -> bool {
            v2vService->followerStatus();
            v2vService->leaderStatus(pedalPos, steeringAngle, 0);
            return true;
        }};
        od4->timeTrigger(FREQ, atFrequency);
    }
}

/**
 * Implementation of the V2VService class as declared in V2VService.hpp
 */
V2VService::V2VService(std::string ip, std::string id) {
    _IP = ip;
    _ID = id;

    /*
     * The broadcast field contains a reference to the broadcast channel which is an OD4Session. This is where
     * AnnouncePresence messages will be received.
     */
    broadcast =
    std::make_shared<cluon::OD4Session>(BROADCAST_CHANNEL,
    [this](cluon::data::Envelope &&envelope) noexcept {
        std::cout << "[OD4] ";
        switch (envelope.dataType()) {
            case ANNOUNCE_PRESENCE: {
                AnnouncePresence ap = cluon::extractMessage<AnnouncePresence>(std::move(envelope));
                std::cout << "received 'AnnouncePresence' from '"
                          << ap.vehicleIp() << "', GroupID '"
                          << ap.groupId() << "'!" << std::endl;

                presentCars[ap.groupId()] = ap.vehicleIp();

                break;
            }
            default: std::cout << "¯\\_(ツ)_/¯" << std::endl;
        }
    });

    /*
     * Each car declares an incoming UDPReceiver for messages directed at them specifically. This is where messages
     * such as FollowRequest, FollowResponse, StopFollow, etc. are received.
     */
    incoming =
    std::make_shared<cluon::UDPReceiver>("0.0.0.0", DEFAULT_PORT,
    [this](std::string &&data, std::string &&sender, std::chrono::system_clock::time_point &&ts) noexcept {
         std::cout << "[UDP] ";
         std::pair<int16_t, std::string> msg = extract(data);

         switch (msg.first) {
             case FOLLOW_REQUEST: {
                 FollowRequest followRequest = decode<FollowRequest>(msg.second);
                 std::cout << "received '" << followRequest.LongName()
                           << "' from '" << sender << "'!" << std::endl;

                 // After receiving a FollowRequest, check first if there is currently no car already following.
                 if (followerIp.empty()) {
                     unsigned long len = sender.find(':');    // If no, add the requester to known follower slot
                     followerIp = sender.substr(0, len);      // and establish a sending channel.
                     toFollower = std::make_shared<cluon::UDPSender>(followerIp, DEFAULT_PORT);
                     followResponse();
                 }
                 break;
             }
             case FOLLOW_RESPONSE: {
                 FollowResponse followResponse = decode<FollowResponse>(msg.second);
                 std::cout << "received '" << followResponse.LongName()
                           << "' from '" << sender << "'!" << std::endl;
                 break;
             }
             case STOP_FOLLOW: {
                 StopFollow stopFollow = decode<StopFollow>(msg.second);
                 std::cout << "received '" << stopFollow.LongName()
                           << "' from '" << sender << "'!" << std::endl;

                 // Clear either follower or leader slot, depending on current role.
                 unsigned long len = sender.find(':');
                 if (sender.substr(0, len) == followerIp) {
                     followerIp = "";
                     toFollower.reset();
                 }
                 else if (sender.substr(0, len) == leaderIp) {
                     leaderIp = "";
                     toLeader.reset();
                 }
                 break;
             }
             case FOLLOWER_STATUS: {
                 FollowerStatus followerStatus = decode<FollowerStatus>(msg.second);
                 std::cout << "received '" << followerStatus.LongName()
                           << "' from '" << sender << "'!" << std::endl;

                 break;
             }
             case LEADER_STATUS: {
                 LeaderStatus leaderStatus = decode<LeaderStatus>(msg.second);
                 std::cout << "received '" << leaderStatus.LongName()
                           << "' from '" << sender << "'!" << std::endl;

                 break;
             }
             default: std::cout << "¯\\_(ツ)_/¯" << std::endl;
         }
    });
}

/**
 * This function sends an AnnouncePresence (id = 1001) message on the broadcast channel. It will contain information
 * about the sending vehicle, including: IP, port and the group identifier.
 */
void V2VService::announcePresence() {
    if (!followerIp.empty()) return;
    AnnouncePresence announcePresence;
    announcePresence.vehicleIp(_IP);
    announcePresence.groupId(_ID);
    od4->send(announcePresence);
    broadcast->send(announcePresence);
}

/**
 * This function sends a FollowRequest (id = 1002) message to the IP address specified by the parameter vehicleIp. And
 * sets the current leaderIp field of the sending vehicle to that of the target of the request.
 *
 * @param vehicleIp - IP of the target for the FollowRequest
 */
void V2VService::followRequest(std::string vehicleIp) {
    if (!leaderIp.empty()) return;
    leaderIp = vehicleIp;
    toLeader = std::make_shared<cluon::UDPSender>(leaderIp, DEFAULT_PORT);
    FollowRequest followRequest;
    followRequest.temporaryValue("Follow request test");
    od4->send(followRequest);
    toLeader->send(encode(followRequest));
}

/**
 * This function send a FollowResponse (id = 1003) message and is sent in response to a FollowRequest (id = 1002).
 * This message will contain the NTP server IP for time synchronization between the target and the sender.
 */
void V2VService::followResponse() {
    if (followerIp.empty()) return;
    FollowResponse followResponse;
    followResponse.temporaryValue("Follow response test");
    od4->send(followResponse);
    toFollower->send(encode(followResponse));
}

/**
 * This function sends a StopFollow (id = 1004) request on the ip address of the parameter vehicleIp. If the IP address is neither
 * that of the follower nor the leader, this function ends without sending the request message.
 *
 * @param vehicleIp - IP of the target for the request
 */
void V2VService::stopFollow(std::string vehicleIp) {
    StopFollow stopFollow;
    stopFollow.temporaryValue("Stop follow test");
    od4->send(stopFollow);
    if (vehicleIp == leaderIp) {
        toLeader->send(encode(stopFollow));
        leaderIp = "";
        toLeader.reset();
    }
    if (vehicleIp == followerIp) {
        toFollower->send(encode(stopFollow));
        followerIp = "";
        toFollower.reset();
    }
}

/**
 * This function sends a FollowerStatus (id = 3001) message on the leader channel.
 *
 * @param speed - current velocity
 * @param steeringAngle - current steering angle
 * @param distanceFront - distance to nearest object in front of the car sending the status message
 * @param distanceTraveled - distance traveled since last reading
 */
void V2VService::followerStatus() {
    if (leaderIp.empty()) return;
    FollowerStatus followerStatus;
    followerStatus.temporaryValue("Follower status test");
    od4->send(followerStatus);
    toLeader->send(encode(followerStatus));
}

/**
 * This function sends a LeaderStatus (id = 2001) message on the follower channel.
 *
 * @param speed - current velocity
 * @param steeringAngle - current steering angle
 * @param distanceTraveled - distance traveled since last reading
 */
void V2VService::leaderStatus(float speed, float steeringAngle, uint8_t distanceTraveled) {
    if (followerIp.empty()) return;
    LeaderStatus leaderStatus;
    leaderStatus.timestamp(getTime());
    leaderStatus.speed(speed);
    leaderStatus.steeringAngle(steeringAngle);
    leaderStatus.distanceTraveled(distanceTraveled);
    od4->send(leaderStatus);
    toFollower->send(encode(leaderStatus));
}

/**
 * Gets the current time.
 *
 * @return current time in milliseconds
 */
uint32_t V2VService::getTime() {
    timeval now;
    gettimeofday(&now, nullptr);
    return (uint32_t ) now.tv_usec / 1000;
}

/**
 * The extraction function is used to extract the message ID and message data into a pair.
 *
 * @param data - message data to extract header and data from
 * @return pair consisting of the message ID (extracted from the header) and the message data
 */
std::pair<int16_t, std::string> V2VService::extract(std::string data) {
    if (data.length() < 10) return std::pair<int16_t, std::string>(-1, "");
    int id, len;
    std::stringstream ssId(data.substr(0, 4));
    std::stringstream ssLen(data.substr(4, 10));
    ssId >> std::hex >> id;
    ssLen >> std::hex >> len;
    return std::pair<int16_t, std::string> (
            data.length() -10 == len ? id : -1,
            data.substr(10, data.length() -10)
    );
};

/**
 * Generic encode function used to encode a message before it is sent.
 *
 * @tparam T - generic message type
 * @param msg - message to encode
 * @return encoded message
 */
template <class T>
std::string V2VService::encode(T msg) {
    cluon::ToProtoVisitor v;
    msg.accept(v);
    std::stringstream buff;
    buff << std::hex << std::setfill('0')
         << std::setw(4) << msg.ID()
         << std::setw(6) << v.encodedData().length()
         << v.encodedData();
    return buff.str();
}

/**
 * Generic decode function used to decode an incoming message.
 *
 * @tparam T - generic message type
 * @param data - encoded message data
 * @return decoded message
 */
template <class T>
T V2VService::decode(std::string data) {
    std::stringstream buff(data);
    cluon::FromProtoVisitor v;
    v.decodeFrom(buff);
    T tmp = T();
    tmp.accept(v);
    return tmp;
}

