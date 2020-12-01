/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#include <iostream>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include "config_reader.h"
#include "simulator.h"
#include "util.h"

/***************************************************************/
/* Simulator.                                                  */
/***************************************************************/
Simulator * simulator;

/***************************************************************/
/*                                                             */
/* Procedure: writeProgramToMem                                */
/*                                                             */
/* Purpose: write the program to the memory                    */
/*                                                             */
/***************************************************************/

void writeProgramToMem(uint32_t address, uint32_t value) {
	int i;
	for (i = 0; i < MEM_NREGIONS; i++) {
		mem_region_t* region = &(simulator->main_memory->MEM_REGIONS[i]);
		if (address >= region->start
				&& address < (region->start + region->size)) {
			uint32_t offset = address - region->start;
			region->mem[offset + 3] = (value >> 24) & 0xFF;
			region->mem[offset + 2] = (value >> 16) & 0xFF;
			region->mem[offset + 1] = (value >> 8) & 0xFF;
			region->mem[offset + 0] = (value >> 0) & 0xFF;
			return;
		}
	}
}

/***************************************************************/
/*                                                             */
/* Procedure: getMemHrchyInfo                                  */
/*                                                             */
/* Purpose: Read memory hierarchy configs                      */
/*                                                             */
/***************************************************************/
MemHrchyInfo* getMemHrchyInfo(char* config_file) {
	FILE * config;
	int ii;
	char line[100];
	config = fopen(config_file, "r");
	assert(config != NULL && "Could not open config file");
	ii = 0;
	std::string str;
	while (fscanf(config, "%s\n", line) != EOF) {
		str.append(line);
	}
	fclose(config);
	ConfigReader msg;
	msg.setJson(str);
	std::cerr << "Config file is read successfully\n";
	MemHrchyInfo * info = new MemHrchyInfo;

	if(msg.getValue("cache_size_l1") != Json::nullValue)
		info->cache_size_l1 = msg.getValue("cache_size_l1").asInt();
	else
		std::cerr << "cache_size_l1 is not defined in config.json, using default value : " << info->cache_size_l1 << "\n";

	if(msg.getValue("cache_assoc_l1") != Json::nullValue)
		info->cache_assoc_l1 = msg.getValue("cache_assoc_l1").asInt();
	else
		std::cerr << "cache_assoc_l1 is not defined in config.json, using default value : " << info->cache_assoc_l1 << "\n";

	if(msg.getValue("cache_size_l2") != Json::nullValue)
		info->cache_size_l2 = msg.getValue("cache_size_l2").asInt();
	else
		std::cerr << "cache_size_l2 is not defined in config.json, using default value : " << info->cache_size_l2 << "\n";

	if(msg.getValue("cache_assoc_l2") != Json::nullValue)
		info->cache_assoc_l2 = msg.getValue("cache_assoc_l2").asInt();
	else
		std::cerr << "cache_assoc_l2 is not defined in config.json, using default value : " << info->cache_assoc_l2 << "\n";

	if(msg.getValue("cache_blk_size") != Json::nullValue)
		info->cache_blk_size = msg.getValue("cache_blk_size").asInt();
	else
		std::cerr << "cache_blk_size is not defined in config.json, using default value : " << info->cache_blk_size << "\n";

	if(msg.getValue("repl_policy_l1d") != Json::nullValue)
		info->repl_policy_l1d = msg.getValue("repl_policy_l1d").asInt();
	else
		std::cerr << "repl_policy_l1d is not defined in config.json, using default value : " << info->repl_policy_l1d << "\n";

	if(msg.getValue("repl_policy_l1i") != Json::nullValue)
		info->repl_policy_l1i = msg.getValue("repl_policy_l1i").asInt();
	else
		std::cerr << "repl_policy_l1i is not defined in config.json, using default value : " << info->repl_policy_l1i << "\n";

	if(msg.getValue("repl_policy_l2") != Json::nullValue)
		info->repl_policy_l2 = msg.getValue("repl_policy_l2").asInt();
	else
		std::cerr << "repl_policy_l2 is not defined in config.json, using default value : " << info->repl_policy_l2 << "\n";

	if(msg.getValue("access_delay_l1") != Json::nullValue)
		info->access_delay_l1 = msg.getValue("access_delay_l1").asInt();
	else
		std::cerr << "access_delay_l1 is not defined in config.json, using default value : " << info->access_delay_l1 << "\n";

	if(msg.getValue("access_delay_l2") != Json::nullValue)
		info->access_delay_l2 = msg.getValue("access_delay_l2").asInt();
	else
		std::cerr << "access_delay_l2 is not defined in config.json, using default value : " << info->access_delay_l2 << "\n";

	if(msg.getValue("memDelay") != Json::nullValue)
		info->memDelay = msg.getValue("memDelay").asInt();
	else
		std::cerr << "memDelay is not defined in config.json, using default value : " << info->memDelay << "\n";

	if(msg.getValue("debugMemory") != Json::nullValue)
		DEBUG_MEMORY = msg.getValue("debugMemory").asBool();
	if(msg.getValue("debugPipe") != Json::nullValue)
		DEBUG_PIPE = msg.getValue("debugPipe").asBool();
	if(msg.getValue("debugCache") != Json::nullValue)
		DEBUG_CACHE = msg.getValue("debugCache").asBool();
	if(msg.getValue("debugPrefetch") != Json::nullValue)
		DEBUG_PREFETCH = msg.getValue("debugPrefetch").asBool();
	if(msg.getValue("debugAll") != Json::nullValue && msg.getValue("debugAll").asBool())
		DEBUG_MEMORY = DEBUG_PIPE = DEBUG_CACHE = DEBUG_PREFETCH = true;
	if(msg.getValue("traceMemory") != Json::nullValue)
		TRACE_MEMORY = msg.getValue("traceMemory").asBool();

	return info;
}

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help() {
	printf("----------------MIPS ISIM Help-----------------------\n");
	printf("go                     -  run program to completion         \n");
	printf("run n                  -  execute program for n instructions\n");
	printf(
			"registerDump                  -  dump architectural registers      \n");
	printf("memDump low high         -  dump memory from low to high      \n");
	printf("input reg_no reg_value - set GPR reg_no to reg_value  \n");
	printf("?                      -  display this help menu            \n");
	printf("quit                   -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : getCommand                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */
/*                                                             */
/***************************************************************/
bool getCommand() {
	char buffer[20];
	int start, stop, cycles;
	int register_no, register_value;

	printf("SIM> ");

	//End of commands
	if (getenv("unattended")){
		printf("g");
		buffer[0]='g';
	}
	else if (scanf("%s", buffer) == EOF){
		return false;
	}

	printf("\n");

	switch (buffer[0]) {
	case 'G':
	case 'g':
		simulator->go();
		break;

	case 'M':
	case 'm':
		if (scanf("%i %i", &start, &stop) != 2)
			break;

		simulator->memDump(start, stop);
		break;

	case '?':
		help();
		break;
	case 'Q':
	case 'q':
		printf("Bye.\n");
		return false;

	case 'R':
	case 'r':
		if (buffer[1] == 'd' || buffer[1] == 'D')
			simulator->registerDump();
		else {
			if (scanf("%d", &cycles) != 1)
				break;
			simulator->run(cycles);
		}
		break;

	case 'I':
	case 'i':
		if (scanf("%i %i", &register_no, &register_value) != 2)
			break;

		printf("%i %i\n", register_no, register_value);
		simulator->pipe->REGS[register_no] = register_value;
		break;

	case 'H':
	case 'h':
		if (scanf("%i", &register_value) != 1)
			break;

		simulator->pipe->HI = register_value;
		break;

	case 'L':
	case 'l':
		if (scanf("%i", &register_value) != 1)
			break;

		simulator->pipe->LO = register_value;
		break;

	default:
		printf("Invalid Command\n");
		break;
	}
	return true;
}

/**************************************************************/
/*                                                            */
/* Procedure : loadProgram                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void loadProgram(char *program_filename) {
	FILE * prog;
	int ii, word;

	/* Open program file. */
	prog = fopen(program_filename, "r");
	if (prog == NULL) {
		printf("Error: Can't open program file %s\n", program_filename);
		exit(-1);
	}

	/* Read in the program. */
	ii = 0;
	while (fscanf(prog, "%x\n", &word) != EOF) {
		writeProgramToMem(MEM_TEXT_START + ii, word);
		ii += 4;
	}

	fclose(prog);

	printf("Read %d words from program into memory.\n\n", ii / 4);
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files) {
	int i;
	for (i = 0; i < num_prog_files; i++) {
		loadProgram(program_filename);
		while (*program_filename++ != '\0')
			;
	}
	simulator->pipe->RUN_BIT = true;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {

	/* Error Checking */
	if (argc < 3) {
		printf(
				"Error: usage: %s <config_file> <program_file_1> <program_file_2> ...\n",
				argv[0]);
		exit(1);
	}
	MemHrchyInfo* info = getMemHrchyInfo(argv[1]);
	printf("Simulator...\n\n");

	simulator = new Simulator(info);
	initialize(argv[2], argc - 2);
	while (getCommand())
		;

	delete (simulator);
	delete (info);
	return 0;
}
