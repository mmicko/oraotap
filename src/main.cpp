#include <cstring>
#include "cxxopts.hpp"
#include "AudioFile.h"

int main(int argc, const char* argv[])
{
	AudioFile<double> audioFile;
	FILE *fp;
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
			("help", "Print help")
		;

		options.parse_positional({"input", "output"});

		auto result = options.parse(argc, argv);

		if (result.count("help") || result.arguments().empty())
		{
			std::cout << options.help({"", "Group"}) << std::endl;
			exit(0);
		}

		if (result.count("input"))
		{
			audioFile.load(result["input"].as<std::string>());
		} else {
			std::cerr << "Input file is mandatory." << std::endl;
			exit(1);
		}

		if (result.count("output"))
		{
			fp = fopen(result["output"].as<std::string>().c_str(), "wb");
		} else {
			std::cerr << "Output file is mandatory." << std::endl;	
			exit(1);
		}

	}
	catch (const cxxopts::OptionException& e)
	{
		std::cerr << "error parsing options: " << e.what() << std::endl;
		exit(1);
	}

	std::cout << "Converting Orao WAV tape to TAP format." << std::endl;
	audioFile.printSummary();
	int numSamples = audioFile.getNumSamplesPerChannel();
	int numChannels = audioFile.getNumChannels();
	 
	int cnt = 0;
	bool firstPart = false;
	uint8_t bit = 0;
	uint8_t prevVal = 0;
	uint8_t bitcounter = 0;
	uint8_t data = 0x4f;
	// Make first byte same as in tool from Josip
	fwrite(&data, 1, 1, fp);
	for (long i = 0; i < numSamples; i++)
	{
		double currentSample = audioFile.samples[0][i];
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