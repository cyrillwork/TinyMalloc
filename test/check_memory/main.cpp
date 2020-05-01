#include <stdio.h>
#include <memory>
#include <vector>

#include "profiler.h"

#ifdef USE_TINYMALLOC
	extern "C" {
		#include "include/malloc.h"
	}
#endif


class A {
public:
	A(){
		std::fill_n(buff, sizeof (buff), 0);
	}
private:
	char buff[128];
};


int main()
{
	std::cout << "Check memory" << std::endl;

#ifdef USE_TINYMALLOC
	InitTinyMalloc(1000000000, 0);
#endif

	{
		char* ptr1 = (char*)malloc(50000000);
		char* ptr2 = (char*)malloc(150000000);

		printf("ptr1=%p\n", ptr1);
		printf("ptr2=%p\n", ptr2);

		if(ptr1){
			free(ptr1);
		}

		if(ptr2){
			free(ptr2);
		}

#ifdef USE_TINYMALLOC
		DumpTinyMalloc("dump1.txt");
#endif

	}

	A *a = new A();
	delete a;

	auto array1 = new A[10]();
	delete [] array1;

#ifdef USE_TINYMALLOC
	DumpTinyMalloc("dump2.txt");
#endif
	{
		Profiler profiler(true);
		std::vector<std::unique_ptr<A>> array;

		for(int i = 0; i <10000; ++i) {
			array.push_back(std::make_unique<A>());
		}

#ifdef USE_TINYMALLOC
		DumpTinyMalloc("dump3.txt");
#endif

	}

#ifdef USE_TINYMALLOC
	DumpTinyMalloc("dump_last.txt");
#endif

	return 0;
}
