#include <algorithm>
#include <chrono>
#include <climits>
#include <iostream>
#include <mutex>
#include <thread>

#include <boost/dynamic_bitset.hpp>

inline uint64_t pow(uint16_t base, uint16_t exp) {
	uint64_t result = 1;
	while (exp) {
		if (exp & 1)
			result *= base;
		exp >>= 1;
		base *= base;
	}
	return result;
}

using Color = uint32_t;
using Pattern = std::vector<Color>;
using PatternSet = boost::dynamic_bitset<>;

struct Correction {
	uint32_t red = 0;
	uint32_t white = 0;
};

bool precomputed;
uint32_t patternLength;
uint64_t patternPossibilites;
uint32_t colorNumber;
uint64_t globalAttempt = 0;
uint64_t globalSum = 0;
uint32_t nmbrThreads;
uint32_t nmbrPerThread;
uint64_t nmbrTotal;

std::vector<Pattern> patternList;

inline uint64_t rand(uint64_t& seed) {
	seed = (6364136223846793005 * seed) + 1442695040888963407;
	return seed;
}

Pattern patternFromId(uint64_t id, bool forceComputing = false) {
	if (!forceComputing && precomputed)
		return patternList[id];

	Pattern result { };
	auto _id { id };
	for (uint16_t i = 0; i < patternLength; i++) {
		Color c = _id % colorNumber;
		result.push_back(c);
		_id -= c;
		_id /= colorNumber;
	}
	return result;
}

void precomputePatterns() {
	std::cout << "Precomputing patterns..." << std::endl;
	std::cout.precision(3);

	patternList.clear();
	patternList.reserve(patternPossibilites);
	for (size_t id = 0; id < patternPossibilites; id++) {
		if (id % (patternPossibilites / int(log(patternPossibilites))) == 0)
			std::cout << "\r" << std::fixed << float(100 * id) / float(patternPossibilites) << "%" << std::flush;
		patternList.push_back(patternFromId(id, true));
	}
	std::cout << "\rDone!   " << std::endl;
}

void printPattern(Pattern& pattern) {
	for (auto& e: pattern)
		std::cout << e << " ; ";
}

Pattern createSolution(uint64_t& seed) {
	uint64_t id = rand(seed) % patternPossibilites;
	return patternFromId(id);
}

Pattern getTrial(PatternSet& set, uint64_t& seed) {
	uint64_t id = set.find_first();
	uint64_t idx_set = rand(seed) % set.count();
	for (auto i = 0; i < idx_set; i++) // select a random configuration among remaining ones
		id = set.find_next(id);
	return patternFromId(id);
}

Correction correct(Pattern& solution, Pattern& trial) {
	Correction corr;

	std::vector<uint16_t> colorCountSln(colorNumber);
	std::vector<uint16_t> colorCountTrial(colorNumber);

	for (uint16_t i = 0; i < patternLength; i++) {
		auto colorA = solution[i], colorB = trial[i];
		if (colorA == colorB)
			corr.red++;
		else {
			colorCountSln[colorA]++;
			colorCountTrial[colorB]++;
		}
	}

	for (uint16_t color = 0; color < colorNumber; color++) {
		corr.white += std::min(colorCountSln[color], colorCountTrial[color]);
	}

	return corr;
}

void learn(Pattern& trial, Correction& correction, PatternSet& set) {
	uint64_t bitIndex = set.find_first();
	do {
		Pattern solution = patternFromId(bitIndex);
		Correction corr = correct(solution, trial);
		if (corr.red != correction.red || corr.white != correction.white)
			set.set(bitIndex, false);

		bitIndex = set.find_next(bitIndex);
	} while (bitIndex < patternPossibilites);
}


void solver() {
	uint64_t seed = std::hash<std::thread::id>()(std::this_thread::get_id()) + std::chrono::high_resolution_clock::now().time_since_epoch().count();

	Pattern trial { };
	PatternSet possiblePatterns(patternPossibilites);
	possiblePatterns.set();
	Correction corr;

	std::cout << "When a pattern is proposed, enter the number of red pegs, add a whitespace, then give the number of white pegs." << std::endl;

	do {
		trial = getTrial(possiblePatterns, seed);
		std::cout << "Try this pattern: ";
		printPattern(trial);
		std::cin >> corr.red;
		std::cin >> corr.white;
		learn(trial, corr, possiblePatterns);
		std::cout << "Possibilities left: " << possiblePatterns.count() << std::endl;
		if (possiblePatterns.count() == 0) {
			std::cout << "This is impossible, you gave a wrong information at some point." << std::endl;
		}
	} while (corr.red != patternLength);
}


std::mutex mtx;

void threadLoop() {
	uint64_t seed = std::hash<std::thread::id>()(std::this_thread::get_id()) + std::chrono::high_resolution_clock::now().time_since_epoch().count();
	PatternSet possiblePatterns(patternPossibilites);

	Pattern solution { };
	Pattern trial { };
	Correction answer;

	while( nmbrTotal > 0 ) {
		mtx.lock();
		uint64_t nmbrAttempt = std::min((uint64_t)nmbrPerThread, nmbrTotal);
		nmbrTotal -= nmbrAttempt;
		mtx.unlock();

		uint32_t threadSum = 0;

		for(int32_t it = 0; it < nmbrPerThread; it++) {
			uint32_t threadNmbrTrials = 0;
			solution = createSolution(seed);
			possiblePatterns.set();

			while (true) {
				threadNmbrTrials++;
				trial = getTrial(possiblePatterns, seed);
				answer = correct(solution, trial);
				if (answer.red == patternLength)
					break;
				learn(trial, answer, possiblePatterns);
			}
			threadSum += threadNmbrTrials;
		}

		mtx.lock();
		globalSum += threadSum;
		globalAttempt += nmbrAttempt;
		std::cout << "\rNumber of trials: " << globalAttempt << " -- Average : " << std::fixed << float(globalSum) / float(globalAttempt) << std::flush;
		mtx.unlock();
	}
}

int main() {
	char mode;

	do {
		std::cout << "Do you want to do a benchmark or to use the solver ? Enter B or S :";
		std::cin >> mode;
	} while (mode != 'B' && mode != 'S');

	std::cout << "Enter the number of pegs : ";
	std::cin >> patternLength;
	std::cout << "Enter the number of colors : ";
	std::cin >> colorNumber;

	patternPossibilites = pow(colorNumber, patternLength);

	if (mode == 'S') {
		solver();
	}
	else if (mode == 'B') {
		std::cout << "Use precomputed patterns (use more memory, number of colors to the power of the number of pegs)? If not, it will compute them everytime (more time consuming). Enter 1 for yes, 0 for no: ";
		std::cin >> precomputed;
		std::cout << "Number of threads: ";
		std::cin >> nmbrThreads;
		std::cout << "Batch size per thread: ";
		std::cin >> nmbrPerThread;
		std::cout << "Number of games in total: ";
		std::cin >> nmbrTotal;

		if (precomputed)
			precomputePatterns();

		std::vector<std::thread> threads;

		for (size_t i = 0; i < nmbrThreads; i++)
			threads.push_back( std::thread( threadLoop ) );

		auto startTime = std::chrono::high_resolution_clock::now();

		for (auto& thread : threads)
			thread.join( );

		auto endTime = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(endTime - startTime);

		std::cout.precision( 15 );
		std::cout << std::endl << duration.count( ) << " sec" << std::endl;

	}

	return 0;
}