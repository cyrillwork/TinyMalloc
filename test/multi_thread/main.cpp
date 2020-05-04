#include <stdio.h>
#include <memory>
#include <vector>
#include <new> // Необходим для использования placement new
#include <thread>
#include <string.h>

#include "profiler.h"

#ifdef USE_TINYMALLOC
	extern "C" {
		#include "include/malloc.h"
	}
#endif


class MyThread {
public:
	MyThread (const std::string &n):
		name{n}, ptr{nullptr}, counter{0} {
		//ptr = (char*)malloc(1000);
	}
	~MyThread() { if(ptr) free(ptr); }

	void operator()(void){
		while(true){

			if(ptr) {
				if(strcmp(ptr, name.c_str()) != 0){
					std::cout << "Error " << name << std::endl;
				}
				free(ptr);
			}

			if(counter > 100) {
				counter = 0;
				std::cout << "realloc "<< name << std::endl;
				ptr = (char*)realloc(ptr, 5648);
			} else {
				ptr = (char*)malloc(name.size() + 1);
			}


			strcpy(ptr, name.c_str());

			std::cout << name << std::endl;
			++counter;

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}
private:
	std::string name;
	char *ptr;
	int counter;
};


int main()
{
	std::cout << "Check memory begin" << std::endl;

	MyThread o1("thread 1");
	MyThread o2("thread 2");
	MyThread o3("thread 3");
	MyThread o4("thread 4");
	MyThread o5("thread 5");

	std::thread t1(o1);
	std::thread t2(o2);
	std::thread t3(o3);
	std::thread t4(o4);
	std::thread t5(o5);

	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();

//#ifdef USE_TINYMALLOC
//        DumpTinyMalloc("dump0_1.txt");
//#endif

	std::cout << "Check memory end" << std::endl;

	return 0;
}
