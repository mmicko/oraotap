#include <cstring>
#include <fstream>
#include "cxxopts.hpp"
#include "wave/file.h"

#define ORAO_WAVE_ONE   17
#define ORAO_WAVE_ZERO  9
#define WAVE_HIGH       -24576
#define WAVE_LOW        24576
#define ORAO_HEADER_SIZE 360

void orao_output_wave(std::vector<float> *content, float value, int repeat)
{
	for (int i=0;i<repeat;i++)
		content->push_back(value);
}

uint8_t reverse(uint8_t b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

int main(int argc, const char* argv[])
{
	std::string input_file;
	std::string output_file;
	bool wav_to_tap = false;
	bool wav_to_oldtap = false;
	bool tap_to_wav = false;
	bool old_to_new = false;
	bool new_to_old = false;
	try
	{
		std::string name = argv[0];
		cxxopts::Options options(name.c_str(), name + " - Orao cassette tape tool");
		options
			.positional_help("[optional args]")
			.show_positional_help();

		options
			.allow_unrecognised_options()
			.add_options()
			("i,input", "Input", cxxopts::value<std::string>())
			("o,output", "Output file", cxxopts::value<std::string>())
			("tap", "Convert WAV to TAP", cxxopts::value<bool>())
			("oldtap", "Convert WAV to oldTAP", cxxopts::value<bool>())
			("wav", "Convert TAP to WAV", cxxopts::value<bool>())
			("old2new", "Convert old TAP to new TAP", cxxopts::value<bool>())
			("new2old", "Convert new TAP to old TAP <WARNING: header will not be good>", cxxopts::value<bool>())
			("help", "Print help")
		;

		options.parse_positional({"input", "output"});

		auto result = options.parse(argc, argv);

		if (result.count("help") || result.arguments().empty())
		{
			std::cout << options.help() << std::endl;
			exit(0);
		}

		if (result.count("input"))
		{
			input_file = result["input"].as<std::string>();
		} else {
			std::cerr << "Input file is mandatory." << std::endl;
			return 1;
		}

		if (result.count("output"))
		{
			output_file = result["output"].as<std::string>();
		} else {
			std::cerr << "Output file is mandatory." << std::endl;	
			return 1;
		}

		if (result.count("wav") && result.count("tap") && result.count("old2new") && result.count("new2old") && result.count("oldtap"))
		{
			std::cerr << "Only one target format is possible." << std::endl;	
			return 1;
		}
		if (!result.count("wav") && !result.count("tap") && !result.count("old2new") && !result.count("new2old") && !result.count("oldtap"))
		{
			std::cerr << "Target format is mandatory." << std::endl;	
			return 1;
		}
		if (result.count("wav")) tap_to_wav = true;
		if (result.count("tap")) wav_to_tap = true;
		if (result.count("oldtap")) wav_to_oldtap = true;
		if (result.count("old2new")) old_to_new = true;
		if (result.count("new2old")) new_to_old = true;

	}
	catch (const cxxopts::OptionException& e)
	{
		std::cerr << "Error parsing options: " << e.what() << std::endl;
		return 1;
	}

	if (wav_to_tap) {
		wave::File read_file;
		std::vector<float> content;	
		wave::Error err = read_file.Open(input_file, wave::kIn);
		if (err) {
			std::cerr << "Something went wrong while opening input file." << std::endl;
			return 1;
		}
		err = read_file.Read(&content);
		if (err) {
			std::cout << "Something went wrong while reading input file." << std::endl;
			return 1;
		}
		FILE *fp = fopen(output_file.c_str(), "wb");

		std::cout << "Converting Orao WAV tape to TAP format." << std::endl;
		size_t numSamples = content.size();
		int cnt = 0;
		bool firstPart = false;
		uint8_t bit = 0;
		uint8_t prevVal = 0;
		uint8_t bitcounter = 0;
		uint8_t data = 0x4f;
		// Make first byte same as in tool from Josip
		fwrite(&data, 1, 1, fp);
		for (size_t i = 0; i < numSamples; i++)
		{
			double currentSample = content[i];
			uint8_t val = (currentSample > 0) ? 1 : 0;
			if (prevVal == val)
				cnt++;
			else {
				bit = (cnt>=12) ? 1 : 0;
				if (!firstPart) {
						bit = (cnt>=11) ? 1 : 0;
						data = (data << 1) | bit;
						if (bitcounter == 7) {
							fwrite(&data, 1, 1, fp);
							data = 0;
							bitcounter = 0;
						} else {
							bitcounter++;
						}
					firstPart = true;
				} else {
					firstPart = false;
				}
				cnt = 0;
			}
			prevVal = val;
		}
		if (bitcounter != 0) {
			for(int i=bitcounter; i<7; i++)
				data = (data << 1);
			fwrite(&data, 1, 1, fp);
		}
		fclose(fp);
		return 0;
	}
	if (wav_to_oldtap) {
		wave::File read_file;
		std::vector<float> content;	
		wave::Error err = read_file.Open(input_file, wave::kIn);
		if (err) {
			std::cerr << "Something went wrong while opening input file." << std::endl;
			return 1;
		}
		err = read_file.Read(&content);
		if (err) {
			std::cout << "Something went wrong while reading input file." << std::endl;
			return 1;
		}
		FILE *fp = fopen(output_file.c_str(), "wb");

		std::cout << "Converting Orao WAV tape to old TAP format." << std::endl;
		std::cout << "WARNING: Header is not formed well, so some emulators may not work." << std::endl;
		size_t numSamples = content.size();
		int cnt = 0;
		bool firstPart = false;
		uint8_t bit = 0;
		uint8_t prevVal = 0;
		uint8_t bitcounter = 0;
		uint8_t data = 0x68; fwrite(&data, 1, 1, fp);
		data = 0x01; fwrite(&data, 1, 1, fp);
		data = 0x00; fwrite(&data, 1, 1, fp);
		for (int i = 0; i < ORAO_HEADER_SIZE -3; i++) {
			fwrite(&data, 1, 1, fp); // fill rest of header with 0
		}
		for (size_t i = 0; i < numSamples; i++)
		{
			double currentSample = content[i];
			uint8_t val = (currentSample > 0) ? 1 : 0;
			if (prevVal == val)
				cnt++;
			else {
				bit = (cnt>=12) ? 1 : 0;
				if (!firstPart) {
						bit = (cnt>=11) ? 0x80 : 0;
						data = (data >> 1) | bit;
						if (bitcounter == 7) {
							fwrite(&data, 1, 1, fp);
							data = 0;
							bitcounter = 0;
						} else {
							bitcounter++;
						}
					firstPart = true;
				} else {
					firstPart = false;
				}
				cnt = 0;
			}
			prevVal = val;
		}
		if (bitcounter != 0) {
			for(int i=bitcounter; i<7; i++)
				data = (data >> 1);
			fwrite(&data, 1, 1, fp);
		}
		fclose(fp);
		return 0;
	}
	if (tap_to_wav) {
		wave::File write_file;
		std::vector<float> content;	
		wave::Error err = write_file.Open(output_file, wave::kOut);
		if (err) {
			std::cerr << "Something went wrong while opening output file." << std::endl;
			return 1;
		}
		std::ifstream fs(input_file, std::ios_base::binary);
		if (!fs) {
			std::cerr << "Failed to open input file" << std::endl;
			return 1;
		}
		std::vector<uint8_t> bytes; 
		std::cout << "Converting Orao TAP tape to WAV format." << std::endl;
		fs.seekg(0, fs.end);
		size_t length = size_t(fs.tellg())-1;
		fs.seekg(1, fs.beg);
		bytes.resize(length);
		fs.read(reinterpret_cast<char *>(&(bytes[0])), length);
		for(uint8_t byte : bytes) {
			for (int j=0;j<8;j++) {
				uint8_t b = (byte >> (7-j)) & 1;
				if (b==0) {
					orao_output_wave(&content, WAVE_LOW , ORAO_WAVE_ZERO);
					orao_output_wave(&content, WAVE_HIGH, ORAO_WAVE_ZERO);
				} else {
					orao_output_wave(&content, WAVE_LOW,  ORAO_WAVE_ONE);
					orao_output_wave(&content, WAVE_HIGH, ORAO_WAVE_ONE);
				}
			}
		}
		err = write_file.Write(content);
		if (err) {
			std::cout << "Something went wrong while writing output file." << std::endl;
			return 1;
		}
		return 0;
	}
	if (old_to_new) {
		std::ifstream fs(input_file, std::ios_base::binary);
		if (!fs) {
			std::cerr << "Failed to open input file" << std::endl;
			return 1;
		}
		std::vector<uint8_t> bytes; 
		std::cout << "Converting Orao old TAP tape to new TAP format." << std::endl;
		fs.seekg(0, fs.end);
		size_t length = size_t(fs.tellg());
		fs.seekg(0, fs.beg);
		bytes.resize(length);
		fs.read(reinterpret_cast<char *>(&(bytes[0])), length);
		FILE *fp = fopen(output_file.c_str(), "wb");
		uint8_t data = 0x4f;
		// Make first byte same as in tool from Josip
		fwrite(&data, 1, 1, fp);
		for (int i = ORAO_HEADER_SIZE; i < bytes.size(); i++) {
			data = reverse(bytes[i]);
			fwrite(&data, 1, 1, fp);
		}
	}
	if (new_to_old) {
		std::ifstream fs(input_file, std::ios_base::binary);
		if (!fs) {
			std::cerr << "Failed to open input file" << std::endl;
			return 1;
		}
		std::vector<uint8_t> bytes; 
		std::cout << "Converting Orao new TAP tape to old TAP format." << std::endl;
		std::cout << "WARNING: Header is not formed well, so some emulators may not work." << std::endl;
		fs.seekg(0, fs.end);
		size_t length = size_t(fs.tellg());
		fs.seekg(0, fs.beg);
		bytes.resize(length);
		fs.read(reinterpret_cast<char *>(&(bytes[0])), length);
		FILE *fp = fopen(output_file.c_str(), "wb");
		uint8_t data = 0x68; fwrite(&data, 1, 1, fp);
		data = 0x01; fwrite(&data, 1, 1, fp);
		data = 0x00; fwrite(&data, 1, 1, fp);
		for (int i = 0; i < ORAO_HEADER_SIZE -3; i++) {
			fwrite(&data, 1, 1, fp); // fill rest of header with 0
		}
		for (int i = 1; i < bytes.size(); i++) {
			data = reverse(bytes[i]);
			fwrite(&data, 1, 1, fp);
		}
	}
	return 0;
}
