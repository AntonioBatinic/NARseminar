#include <iostream>
#include <chrono>
#include <thread>


using namespace std;
double nesto=0;

void whack_cache(const int sz)
{
	int* arr = new int[sz];

	int steps = 64 * 1024 * 1024;

	auto before = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < steps; i++)
	{
		arr[(i * 16) % sz]++;
	}

	auto time = std::chrono::high_resolution_clock::now() - before;
	nesto = arr[0];
	delete[] arr;
	cout << time.count() << endl;
}

int main()
{
	cout << "Started" << endl;

	for (int sz = 256; sz <= 64 * 1024 * 1024; sz <<= 1)
	{
		cout << (float)sz / 256 << " kB\t\t";
		thread th(whack_cache, sz);
		th.join();
	}
	cout << "done";
	char c;
	cin >> c;
	cout << nesto;

	return 0;
}