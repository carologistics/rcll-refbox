
/***************************************************************************
 *  rcll-set-lights.cpp - set MPS lights
 *
 *  Created: Mon Mar 21 17:18:21 2016
 *  Copyright  2013-2016  Tim Niemueller [www.niemueller.de]
 *                  2017  Tobias Neumann
 ****************************************************************************/

/*  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * - Neither the name of the authors nor the names of its contributors
 *   may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define BOOST_DATE_TIME_POSIX_TIME_STD_CONFIG

#include <config/yaml.h>

#include <protobuf_comm/server.h>
#include <utils/system/argparser.h>

#include <msgs/MachineInstructions.pb.h>
#include <msgs/MachineDescription.pb.h>

using namespace protobuf_comm;
using namespace llsf_msgs;
using namespace fawkes;

boost::asio::io_service io_service_;

static bool quit = false;
ProtobufStreamServer *server_ = NULL;

std::string machine_name_;

void
signal_handler(const boost::system::error_code& error, int signum)
{
  if (!error) {
    quit = true;
    io_service_.stop();
  }
}

void
handle_connected(ProtobufStreamServer::ClientID client,
                 boost::asio::ip::tcp::endpoint &endpoint)
{
  printf("Connected to RefBox\n");
//  llsf_msgs::SetMachineLights set_lights;
//  set_lights.set_machine_name(machine_name_);

//  llsf_msgs::LightSpec *red = set_lights.add_lights();
//  red->set_color(llsf_msgs::RED);
//  red->set_state(light_state_red_);
  
//  llsf_msgs::LightSpec *yellow = set_lights.add_lights();
//  yellow->set_color(llsf_msgs::YELLOW);
//  yellow->set_state(light_state_yellow_);

//  llsf_msgs::LightSpec *green = set_lights.add_lights();
//  green->set_color(llsf_msgs::GREEN);
//  green->set_state(light_state_green_);

//  printf("Message:\n"
//         "==================================================================\n"
//         "%s\n"
//         "==================================================================\n",
//         set_lights.DebugString().c_str());
  
//  client_->send(set_lights);

//  usleep(200000);
//  quit = true;
//  io_service_.stop();
}

void
handle_disconnected(ProtobufStreamServer::ClientID client,
                    const boost::system::error_code &error)
{
//  quit = true;
//  io_service_.stop();
}

void
handle_receive_failed(ProtobufStreamServer::ClientID client,
                      uint16_t component_id, uint16_t msg_type,
                      std::string msg)
{
  printf("Received failed to %u: comp_id: %u, msg_type: %u\n", client, component_id, msg_type);
}

void
handle_message(ProtobufStreamServer::ClientID client,
               uint16_t component_id, uint16_t msg_type,
               std::shared_ptr<google::protobuf::Message> msg)
{
  bool reply = false;
  unsigned int id = -1;
  std::string machine = "NOT-SET";
  std::shared_ptr<llsf_msgs::InstructMachine> im;
  if ( (im = std::dynamic_pointer_cast<llsf_msgs::InstructMachine>(msg)) ) {
    id = im->id();
    machine = im->machine();
    if (machine == machine_name_) { // if this machine is running here

      reply = true;

      // generic stuff for machines
      switch ( im->set() ) {
        case llsf_msgs::INSTRUCT_MACHINE_SET_SIGNAL_LIGHT:
          printf("Set light to: (ryg): %s, %s, %s\n",
                 llsf_msgs::LightState_Name( im->light_state().red() ).c_str(),
                 llsf_msgs::LightState_Name( im->light_state().yellow() ).c_str(),
                 llsf_msgs::LightState_Name( im->light_state().green() ).c_str()
                );
          break;
        case llsf_msgs::INSTRUCT_MACHINE_MOVE_CONVEYOR:
          printf("Move conveyor to %s and stop at %s\n",
                 llsf_msgs::ConveyorDirection_Name( im->conveyor_belt().direction() ).c_str(),
                 llsf_msgs::SensorOnMPS_Name( im->conveyor_belt().stop_sensor() ).c_str()
                );
          sleep(2);
          break;
        case llsf_msgs::INSTRUCT_MACHINE_STOP_CONVEYOR:
          printf("STOP! conveyor\n");
          sleep(1);
          break;
        case llsf_msgs::INSTRUCT_MACHINE_WAIT_FOR_PICKUP:
          printf("Wait for pickup at TODO-fix-msg-and-send-sensor grace-time is TODO-add change light to (ryg) %s %s %s in case the product is picked up\n",
                 llsf_msgs::LightState_Name( im->light_state().red() ).c_str(),
                 llsf_msgs::LightState_Name( im->light_state().yellow() ).c_str(),
                 llsf_msgs::LightState_Name( im->light_state().green() ).c_str()
                );
          sleep(2);
          break;
        default:
          // now do the specifig stuff for the machines
          if ( im->machine().find("BS") != std::string::npos ) {  // if it is a BS
            switch ( im->set() ) {
              case llsf_msgs::INSTRUCT_MACHINE_BS:
                printf("Pushout from feeder # %u\n",
                       im->bs().slot()
                      );
                sleep(1);
                break;
              default:
                printf("Error, unknown \"set\": %u\n", im->set());
                reply = false;
                break;
            }
          } else if ( im->machine().find("SS") != std::string::npos ) {

            switch ( im->set() ) {
              case llsf_msgs::INSTRUCT_MACHINE_SS:
                printf("%s from slot %u %u %u\n",
                       llsf_msgs::SSOp_Name( im->ss().operation() ).c_str(),
                       im->ss().slot().x(),
                       im->ss().slot().y(),
                       im->ss().slot().z()
                      );
                sleep(5);
                break;
              default:
                printf("Error, unknown \"set\": %u\n", im->set());
                reply = false;
                break;
            }
          } else if ( im->machine().find("DS") != std::string::npos ) {

            switch ( im->set() ) {
              case llsf_msgs::INSTRUCT_MACHINE_DS:
                printf("Set to gate %u\n", im->ds().gate() );
                sleep(1);
                break;
              default:
                printf("Error, unknown \"set\": %u\n", im->set());
                reply = false;
                break;
            }
          } else if ( im->machine().find("CS") != std::string::npos ) {

            switch ( im->set() ) {
              case llsf_msgs::INSTRUCT_MACHINE_CS:
                printf("%s cap\n", llsf_msgs::CSOp_Name( im->cs().operation() ).c_str() );
                sleep(2);
                break;
              default:
                printf("Error, unknown \"set\": %u\n", im->set());
                reply = false;
                break;
            }
          } else if ( im->machine().find("RS") != std::string::npos ) {

            switch ( im->set() ) {
              case llsf_msgs::INSTRUCT_MACHINE_RS:
                printf("Mount ring from feeder %u\n", im->rs().feeder() );
                sleep(2);
                break;
              default:
                printf("Error, unknown \"set\": %u\n", im->set());
                reply = false;
                break;
            }
          }
      }

      if ( reply ) {
        llsf_msgs::MachineReply reply;
        reply.set_id( id );
        reply.set_machine( im->machine() );
        reply.set_set( llsf_msgs::MACHINE_REPLY_FINISHED );
        server_->send_to_all(reply);
      }
    }
  }
}



void
usage(const char *progname)
{
  printf("Usage: %s [-R host[:port]] -m <machine-name>\n",
//  printf("Usage: %s [-R host[:port]]\n",
	 progname);
}


int
main(int argc, char **argv)
{
  ArgumentParser argp(argc, argv, "m:R");
//  ArgumentParser argp(argc, argv, "R");

  if ( ! (argp.has_arg("m")) ) {
    usage(argv[0]);
    exit(1);
  }
  machine_name_ = argp.arg("m");

  llsfrb::YamlConfiguration *config_ = new llsfrb::YamlConfiguration(CONFDIR);
  config_->load("config.yaml");
  std::string cfg_prefix = std::string("/llsfrb/mps/stations/");
//  std::string host = config_->get_string( (cfg_prefix + machine_name_ + "/host").c_str() );
  unsigned int port = config_->get_uint( (cfg_prefix + machine_name_ + "/port").c_str() );

  MessageRegister * message_register = new MessageRegister();
  message_register->add_message_type<llsf_msgs::InstructMachine>();
  server_ = new ProtobufStreamServer(port, message_register);

  server_->signal_connected().connect(handle_connected);
  server_->signal_disconnected().connect(handle_disconnected);
  server_->signal_received().connect(handle_message);
  server_->signal_receive_failed().connect(handle_receive_failed);

#if BOOST_ASIO_VERSION >= 100601
  // Construct a signal set registered for process termination.
  boost::asio::signal_set signals(io_service_, SIGINT, SIGTERM);

  // Start an asynchronous wait for one of the signals to occur.
  signals.async_wait(signal_handler);
#endif

  do {
    io_service_.run();
    io_service_.reset();
  } while (! quit);

  delete server_;

  // Delete all global objects allocated by libprotobuf
  google::protobuf::ShutdownProtobufLibrary();
}