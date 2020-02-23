#ifndef _LASER_CONTROLLER_H_
#define _LASER_CONTROLLER_H_
#include <atomic>
#include <string>
#include <thread>
#include <vector>

class LaserController
{
public:
	enum class ReturnCodes
	{
		success,								//success
		failure,								//failure
		unkown_command,							//unkown_command
	};
	const std::string m_unkown_command{ "UK!" };
	const std::string m_success{ "#" };
	const std::string m_failure{ "!" };
	const char m_delimiter = '|';
	const static int m_required_response_buffer_size = 256;


	LaserController();
	~LaserController();
	ReturnCodes Process_Command(const char * command, int command_length, char* response, int response_length); // Engine should allocate the buffer, we're a simple library

private:

	const std::vector<std::string> m_known_commands_strings = { "STR", "STP", "ST?", "KAL", "PW?", "PW=", "ESM", "DSM" };
	//const std::vector<std::string> m_known_commands_strings_reversed = { "RST", "PTS", "?TS", "LAK", "?WP", "=WP", "MSE", "MDS" };
	//const std::vector<std::string>* m_known_commands_strings{ &m_known_commands_strings_normal };
	std::thread m_emission_thread;
	std::thread m_keep_alive_thread;

	std::atomic<int> m_emission_power_level{ 1 }; //	Assumed default
	std::atomic<bool> m_emission_is_on{ false };
	std::atomic<bool> m_emission_keep_alive{ false };
	std::atomic<bool> m_class_thread_kill_switch{ false };
	std::atomic<bool> m_laser_worker_keep_alive_signal_received{ false };

	const double m_timeout_period = 5000.f;


	enum KnownCommands
	{
		STR,								//STR
		STP,								//STP
		STq,								//ST?
		KAL,								//KAL
		PWq,								//PW?
		PWe,								//PW=
		ESM,								//ESM
		DSM,								//DSM
	};
	bool m_is_silly{ false };


	std::vector<std::string> Split_String(const std::string& string_to_split, const char delimiter); // Helper function, should be in a helper module.
	void Start_Emission();
	void Stop_Emission();
	void Set_Emission_Power(int power_level);
	int Get_Emission_Power();
	int Is_Laser_Emitting();
	void Activate_Silly_Mode();
	void Deactivate_Silly_Mode();
	void Laser_Emission_Worker();
	void Keep_Alive_Worker();
	void Send_Keep_Alive_Signal();
	void Emit();
};

#endif