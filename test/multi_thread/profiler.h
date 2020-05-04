#ifndef PROFILER_H
#define PROFILER_H

#include <iostream>
#include <chrono>

using namespace std::chrono;

class Profiler
{
public:
    Profiler(bool endPrint = false):
        isPrintEnd(endPrint),
        start_point(high_resolution_clock::now())
    {}

    ~Profiler();

    void start();

    void stop();

    friend std::ostream& operator << (std::ostream& s, const Profiler &p)
    {
        s.width(12);
        s.precision(7);
        s << std::fixed << p.time_span.count() << " sec";
        return s;
    }

private:
    bool isPrintEnd;
    high_resolution_clock::time_point start_point;
    duration<double> time_span = {};
};


#endif // PROFILER_H
