#include "LaserController.h"
#include <algorithm>
#include <assert.h>
#include <chrono>
#include <cstring>
#include <sstream>

LaserController::LaserController()
{
	// This thread will keep emitting the laser until stopped.
	m_emission_thread = std::thread(&LaserController::Laser_Emission_Worker, this);
	// A thread that checks for keep alive messages, it kills the laser thread if it doesn't get a keep-alive message every m_timeout_period miliseconds.
	m_keep_alive_thread = std::thread(&LaserController::Keep_Alive_Worker, this); 
}

LaserController::~LaserController()
{
	m_class_thread_kill_switch = true;
	m_keep_alive_thread.join();
	m_emission_thread.join();
}


void LaserController::Keep_Alive_Worker()
{

	while (!m_class_thread_kill_switch)
	{
		while (!m_emission_is_on && !m_class_thread_kill_switch) // Off state
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
		auto latest_keep_alive = std::chrono::high_resolution_clock::now();
		while (m_emission_is_on && !m_class_thread_kill_switch) // On state
		{
			auto now = std::chrono::high_resolution_clock::now();
			if (m_laser_worker_keep_alive_signal_received)
			{
				latest_keep_alive = std::chrono::high_resolution_clock::now();
				m_laser_worker_keep_alive_signal_received = false;
			}
			std::chrono::duration<double, std::milli> time_since_keep_alive = now - latest_keep_alive;
			if (time_since_keep_alive.count() > m_timeout_period)
			{
				Stop_Emission();
				break;
			}
		}
	}
	printf("goodbye from Keep_Alive_Worker()");
}

std::vector<std::string> split(std::string strToSplit, char delimeter)
{
	std::stringstream ss(strToSplit);
	std::string item;
	std::vector<std::string> splittedStrings;
	while (std::getline(ss, item, delimeter))
	{
		splittedStrings.push_back(item);
	}
	return splittedStrings;
}

std::vector<std::string> LaserController::Split_String(const std::string& string_to_split,const char delimiter)
{
	std::stringstream sstream(string_to_split);
	std::string item;
	std::vector<std::string> splittedStrings;
	while (std::getline(sstream, item, delimiter))
	{
		splittedStrings.push_back(item);
	}
	return splittedStrings;
}

void LaserController::Start_Emission()
{
	m_emission_is_on = true;
}

void LaserController::Stop_Emission()
{
	m_emission_is_on = false;
}

void LaserController::Set_Emission_Power(int power_level)
{
	m_emission_power_level=power_level;
}

int LaserController::Get_Emission_Power()
{
	return m_emission_power_level;
}

int LaserController::Is_Laser_Emitting()
{
	return m_emission_is_on;
}

void LaserController::Activate_Silly_Mode()
{
	m_is_silly = true;
}

void LaserController::Deactivate_Silly_Mode()
{
	m_is_silly = false;
}

void LaserController::Laser_Emission_Worker()
{
	while (!m_class_thread_kill_switch)
	{
		while (!m_emission_is_on && !m_class_thread_kill_switch) // Off state
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
		while (m_emission_is_on && !m_class_thread_kill_switch) // On state
		{
			Emit();
		}
	}
}

void LaserController::Emit()
{
	// Laser emissions go here.
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	//printf("emitting...");
}


void LaserController::Send_Keep_Alive_Signal()
{
	m_laser_worker_keep_alive_signal_received = true;
}

LaserController::ReturnCodes LaserController::Process_Command(const char* input_command, int command_length, char* response, int response_length) // Engine should allocate the buffer, we're a simple library
{
	assert(response_length == m_required_response_buffer_size);

	std::string proccessed_command(input_command);
	if (m_is_silly)
	{
		std::reverse(proccessed_command.begin(), proccessed_command.end());
	}

	const auto command_split = Split_String(proccessed_command, m_delimiter);
	if (!command_split.size())
	{
		strcpy(response, m_unkown_command.c_str());
		return ReturnCodes::failure;
	}
	const auto foundCommand = std::find(m_known_commands_strings.begin(), m_known_commands_strings.end(), command_split[0]);
	if (foundCommand == m_known_commands_strings.end())
	{
		strcpy(response, m_unkown_command.c_str());
		return ReturnCodes::unkown_command;
	}

	if (m_known_commands_strings[KnownCommands::PWe] == command_split[0])
	{
		if (command_split.size() != 2)
		{
			strcpy(response, std::string(proccessed_command + m_failure).c_str());
			return ReturnCodes::failure;
		}
		int power_level{ 0 };
		try
		{
			power_level = std::stoi(command_split[1]);
		}
		catch (std::invalid_argument const& e)
		{
			strcpy(response, std::string(proccessed_command + m_failure).c_str());
			return ReturnCodes::failure;
		}
		catch (std::out_of_range const& e)
		{
			strcpy(response, std::string(proccessed_command + m_failure).c_str());
			return ReturnCodes::failure;
		}
		if (0 > power_level || 100 < power_level) // Power can be adjusted even when the laser is off, only check for value correctness.
		{
			strcpy(response, std::string(proccessed_command + m_failure).c_str());
			return ReturnCodes::failure;
		}
		Set_Emission_Power(power_level);
		strcpy(response, std::string(proccessed_command + m_success).c_str());
		return ReturnCodes::success;
	}
	else if (command_split.size() != 1)
	{
		strcpy(response, std::string(proccessed_command + m_failure).c_str());
		return ReturnCodes::failure;
	}
	else if (m_known_commands_strings[KnownCommands::STR] == command_split[0])
	{
		if (m_emission_is_on)
		{
			strcpy(response, std::string(proccessed_command + m_failure).c_str());
			return ReturnCodes::failure;
		}
		Start_Emission();
		strcpy(response, std::string(proccessed_command + m_success).c_str());
		return ReturnCodes::success;
	}
	else if (m_known_commands_strings[KnownCommands::STP] == command_split[0])
	{
		if (!m_emission_is_on)
		{
			strcpy(response, std::string(proccessed_command + m_failure).c_str());
			return ReturnCodes::failure;
		}
		Stop_Emission();
		strcpy(response, std::string(proccessed_command + m_success).c_str());
		return ReturnCodes::success;
	}
	else if (m_known_commands_strings[KnownCommands::STq] == command_split[0])
	{
		int is_laser_emitting = Is_Laser_Emitting();
		sprintf(response, "%c%d%s", m_delimiter, is_laser_emitting, m_success.c_str());
		return ReturnCodes::success;
	}
	else if (m_known_commands_strings[KnownCommands::PWq] == command_split[0])
	{
		int emission_power = Get_Emission_Power();
		sprintf(response, "%c%d%s", m_delimiter, emission_power, m_success.c_str());
		return ReturnCodes::success;
	}
	else if (m_known_commands_strings[KnownCommands::ESM] == command_split[0])
	{
		if (m_is_silly)
		{
			strcpy(response, std::string(proccessed_command + m_failure).c_str());
			return ReturnCodes::failure;
		}
		Activate_Silly_Mode();
		strcpy(response, std::string(proccessed_command + m_success).c_str());
		return ReturnCodes::success;
	}
	else if (m_known_commands_strings[KnownCommands::DSM] == command_split[0])
	{
		if (!m_is_silly)
		{
			strcpy(response, std::string(proccessed_command + m_failure).c_str());
			return ReturnCodes::failure;
		}
		Deactivate_Silly_Mode();
		strcpy(response, std::string(proccessed_command + m_success).c_str());
		return ReturnCodes::success;
	}
	else if (m_known_commands_strings[KnownCommands::KAL] == command_split[0])
	{
		if (!m_emission_is_on)
		{
			strcpy(response, std::string(proccessed_command + m_failure).c_str());
			return ReturnCodes::failure;
		}
		Send_Keep_Alive_Signal();
		strcpy(response, std::string(proccessed_command + m_success).c_str());
		return ReturnCodes::success;
	}
	strcpy(response, std::string(proccessed_command + m_failure).c_str());
	return ReturnCodes::failure;
}
