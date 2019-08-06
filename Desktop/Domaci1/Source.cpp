#include <iostream> 
#include <thread> 
#include <ctime> 
#define MULTIPLIER 20000000

using namespace std;
double nesto;

void foo(double x)
{
	double avg;
	x *= 10;
	for (double i = 0; i < x; i++) {
		avg = i/ 2;
	}
	nesto = avg / 10000000;
}

int main()
{
	int i = 0, th1avg=0, th2avg=0, th4avg=0, th8avg=0, th16avg = 0;

	std::thread th[16];

	for (int j = 0; j < 16; j++) {
		th[j] = std::thread(foo, 8000000);
	}

	for (int j = 0; j < 16; j++) {
		th[j].join();
	}

	cout << "Starting 1 thread" << endl;

	while (i++ < 5) {
		clock_t time = clock();

		th[0] = std::thread(foo, 16 * MULTIPLIER);

		th[0].join();

		time = clock() - time;
		th1avg += time;
		cout << i << ". pass time in miliseconds: " << time << endl;
	}

	cout << endl << "1 thread average of 5: " << th1avg/5 << " ms" << endl;
	i = 0;

	cout << endl << "Starting 2 threads" << endl;

	while (i++ < 5) {
		clock_t time = clock();

		for (int j = 0; j < 2; j++) {
			th[j] = std::thread(foo, 8* MULTIPLIER);
		}

		for (int j = 0; j < 2; j++) {
			th[j].join();
		}

		time = clock() - time;

		th2avg += time;
		cout << i << ". pass time in miliseconds: " << time << endl;
	}

	cout << endl << "2 threads average of 5: " << th2avg / 5 << " ms" << endl;

	i = 0;
	cout << endl << "Starting 4 threads" << endl;

	while (i++ < 5) {
		clock_t time = clock();
		for (int j = 0; j < 4; j++) {
			th[j] = std::thread(foo, 4* MULTIPLIER);
		}

		for (int j = 0; j < 4; j++) {
			th[j].join();
		}

		time = clock() - time;

		th4avg += time;
		cout << i << ". pass time in miliseconds: " << time << endl;
	}

	cout << endl << "4 threads average of 5: " << th4avg / 5 << " ms" << endl;

	i = 0;
	cout << endl << "Starting 8 threads" << endl;

	while (i++ < 5) {
		clock_t time = clock();

		for (int j = 0; j < 8; j++) {
			th[j] = std::thread(foo, 2* MULTIPLIER);
		}

		for (int j = 0; j < 8; j++) {
			th[j].join();
		}

		time = clock() - time;

		th8avg += time;
		cout << i << ". pass time in miliseconds: " << time << endl;
	}

	cout << endl << "8 threads average of 5: " << th8avg / 5 << " ms" << endl;

	i = 0;
	cout << endl << "Starting 16 threads" << endl;

	while (i++ < 5) {
		clock_t time = clock();

		for (int j = 0; j < 16; j++) {
			th[j] = std::thread(foo, MULTIPLIER);
		}

		for (int j = 0; j < 16; j++) {
			th[j].join();
		}

		time = clock() - time;

		th16avg += time;
		cout << i << ". pass time in miliseconds: " << time << endl;
	}

	cout << endl << "16 threads average of 5: " << th16avg / 5 << " ms" << endl;

	char c;
	cin >> c;
	cout << nesto;

	return 0;
}