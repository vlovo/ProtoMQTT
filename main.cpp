#include <windows.h>
#include <iostream>

#include "RobotMsg.pb.h"

extern "C"
{

#include "MQTTClient.h"

};



#include "boost/date_time/posix_time/posix_time.hpp"

using namespace boost::posix_time;

void demoPosixTime()
{

    ptime t0 = microsec_clock::local_time();

    std::string  timeString  = to_simple_string(t0);

    std::string  timeIsoString = to_iso_string(t0);

    ptime t1 = time_from_string(timeString);

    ptime t2 = from_iso_string(timeIsoString);

    if( t0 == t1 && t1 == t2 )  std::cout << "t0 ,t1 and t2 are equal"  << "\n";

    ptime t3 =  from_iso_string("20161120T170143.558219");

    if(t3 > t1 )
    {
        std::cout << " t3 is a new message \n";
    }
    else
    {
        std::cout << "t3 is a message from the past \n";
    }

    return ;
}

class RobotCtrl
{
public:


    RobotCtrl(std::string robotName)
    {
        mRobotMessage.set_robotstate(RobotMsg_RobotStates_Connected);
        mRobotMessage.set_devicename(robotName);
        std::string name = mRobotMessage.devicename();

    }

    void setClient(MQTTClient &client)
    {
        mMQTTClient = client;
    }

    ~RobotCtrl() {};

    int move(std::vector<double> position)
    {
        int ret = 0;

        if(position.size() >0)
        {
            std::cout << "move to " << position[0]  << " " << position[1] << "\n";

            mRobotMessage.mutable_position()->Clear();
            mRobotMessage.mutable_position()->Reserve(position.size());

            memcpy(mRobotMessage.mutable_position()->mutable_data(),
                   position.data(),
                   position.size()*sizeof(mRobotMessage.position(0)));

            std::cout <<  "1" <<  mRobotMessage.position(0) << "  2 "  << mRobotMessage.position(1) << "  3 " << mRobotMessage.position(2) << " \n";

        }

        return(ret);

    }

    int publishMessage( )
    {
        ptime t0 = microsec_clock::local_time();
        std::string  timeString  = to_simple_string(t0);

        mRobotMessage.set_timestamp(timeString);

        MQTTClient_deliveryToken dt;
        MQTTClient_message pubmsg = MQTTClient_message_initializer;
        pubmsg.payload = (void*)(mRobotMessage.SerializeAsString()).c_str();
        pubmsg.payloadlen =(int)(mRobotMessage.SerializeAsString()).size();
        pubmsg.retained = 0;

        std::string topic = "Robo/data";
        int rc=0;

        if(nullptr != mMQTTClient)
        {


            rc = MQTTClient_publish(mMQTTClient,
                                    topic.c_str(),
                                    pubmsg.payloadlen,
                                    pubmsg.payload,
                                    pubmsg.qos,
                                    pubmsg.retained,
                                    &dt);

            if (pubmsg.qos > 0)
            {
                rc = MQTTClient_waitForCompletion(mMQTTClient, dt, 5000L);
            }
        }
        else
        {
            rc = -1;
        }

        return (rc);
    }





    static int onMessageArrived(void* context, char* topic , int tlen, MQTTClient_message *msg)
    {

        RobotCtrl *caller = (RobotCtrl*) context;

        std::string _topic(topic);
        RobotMsg _msg;
#ifndef REAL
        _msg.add_position(1);
        _msg.add_position(2);
        _msg.add_position(3);


#else
        _msg.ParseFromArray(msg->payload,msg->payloadlen);
#endif

        std::vector<double> p(_msg.position_size(),0);

        memcpy(p.data(),
               _msg.mutable_position()->mutable_data(),
               _msg.position_size()*sizeof(double));

        caller->move(p);


        return(1);
    }




private:
    RobotMsg mRobotMessage;
    MQTTClient mMQTTClient;
    std::vector<double> mPosition;
};





int setupMQTT(MQTTClient &mqttClient,RobotCtrl &robo)
{

    std::string  clientID=  "RoboClient";

    int rc = MQTTClient_create(&mqttClient, "tcp://127.0.0.1:1883", clientID.c_str(), MQTTCLIENT_PERSISTENCE_DEFAULT, NULL);

    int rc2 = MQTTClient_setCallbacks(mqttClient, &robo,0, RobotCtrl::onMessageArrived ,0);


    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    MQTTClient_willOptions wopts = MQTTClient_willOptions_initializer;

    opts.keepAliveInterval = 20;
    opts.cleansession = 1;
    opts.connectTimeout = 2;
    opts.will = &wopts;
    opts.will->message = "R2D2 disconnected";
    opts.will->qos = 1;
    opts.will->retained = 0;
    opts.will->topicName = "Robo/disconnect";

    rc = MQTTClient_connect(mqttClient, &opts);

    if(rc != MQTTCLIENT_SUCCESS)
    {
        std::cout << " error \n";

    }
    else
    {
        robo.setClient(mqttClient);
    }

    return  rc;
}


int main(int argc, char** argv)
{

    demoPosixTime();


    std::cout << "Hello ProtoMQTT  \n";

    RobotMsg msg;

    std::cout << "msg byte size is " <<  msg.ByteSize() << "\n";
    std::cout << "msg size is "  << sizeof(msg) << "\n";


    ptime t0 = microsec_clock::local_time();
    std::string  timeString  = to_simple_string(t0);
    std::cout << "timeString is " <<timeString.size() <<"\n";

    msg.set_timestamp(timeString);
    std::cout << " msg byte size is " <<  msg.ByteSize() << "\n";






    RobotCtrl robotControler("Kraftwerk");

    MQTTClient mqttClient;

    std::string  clientID=  "RoboClient";

    int rc = MQTTClient_create(&mqttClient, "tcp://127.0.0.1:1883", clientID.c_str(), MQTTCLIENT_PERSISTENCE_DEFAULT, NULL);
    rc = MQTTClient_setCallbacks(mqttClient, &robotControler,0, RobotCtrl::onMessageArrived ,0);


    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    MQTTClient_willOptions wopts = MQTTClient_willOptions_initializer;

    opts.keepAliveInterval = 20;
    opts.cleansession = 1;
    opts.connectTimeout = 2;
    opts.will = &wopts;
    opts.will->message = "R2D2 disconnected";
    opts.will->qos = 1;
    opts.will->retained = 0;
    opts.will->topicName = "Robo/disconnect";

    rc = MQTTClient_connect(mqttClient, &opts);

    if(rc != MQTTCLIENT_SUCCESS)
    {
        std::cout << " error \n";

    }
    else
    {
        robotControler.setClient(mqttClient);

    }


    int ec = MQTTClient_subscribe(mqttClient,"Robo/Input",0);

    int i=0;

    do
    {



        Sleep(500);


        robotControler.publishMessage();

    }
    while (1);





    return 0;
}