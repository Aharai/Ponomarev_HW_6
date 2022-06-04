// Ponomarev_HW_6.cpp

#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <vector>
#include <algorithm>
#include <random>


using namespace std;

static mutex mtx_cout;

class Timer
{
    using clock_t = chrono::high_resolution_clock;
    using second_t = chrono::duration<double, ratio<1>>;

    string m_name;
    chrono::time_point<clock_t> m_beg;
    double elapsed() const
    {
        return chrono::duration_cast<second_t>(clock_t::now() - m_beg).count();
    }

public:
    Timer() : m_beg(clock_t::now()) {}
    Timer(string name) : m_name(name), m_beg(clock_t::now()) {}

    void start(string name)
    {
        m_name = name;
        m_beg = clock_t::now();
    }

    void print() const
    {
        cout << m_name << ":\t" << elapsed() * 1000 << " ms" << endl;
    }
};

//=========================================================================================

class pcout
{
    lock_guard<mutex> lg;

public:
    pcout() : lg(lock_guard<mutex>(mtx_cout)) {}

    template<typename T>
    pcout& operator << (const T& data)
    {
        cout << data;
        return *this;
    }

    pcout& operator << (ostream& (*fp)(ostream&))
    {
        cout << fp;
        return *this;
    }
};

void doSomething(int number)
{
    pcout() << "start thread " << number << endl;
    pcout() << "stop thread " << number << endl;
}

void task_1()
{
    thread th1(doSomething, 1);
    thread th2(doSomething, 2);
    thread th3(doSomething, 3);
    th1.join();
    th2.join();
    th3.join();
}

//=========================================================================================

bool isPrime(size_t num)
{
    size_t limit = num / 2;

    if (num > 2)
    {
        for (size_t i = 2; i <= limit; ++i)
        {
            if (num % i == 0)
            {
                return false;
            }
        }
    }

    return true;
}

void task_2()
{
    size_t maxValue = 500000;
    size_t counThreads = 30;
    vector<size_t> answers;
    mutex m;

    Timer timer("Without threads");

    for (size_t i = 0; i < maxValue;)
    {
        vector<thread> v;

        for (size_t j = i + counThreads; i <= j; ++i)
        {
            v.push_back(thread([=, &m, &answers]()
                {
                    if (isPrime(i))
                    {
                        lock_guard lg(m);
                        answers.push_back(i);
                    }
                }));
        }

        for (auto& t : v)
        {
            t.join();
        }
    }

    timer.print();

    answers.clear();
    answers.shrink_to_fit();

    timer.start("Without threads");

    for (size_t i = 0; i < maxValue; ++i)
    {
        if (isPrime(i))
        {
            answers.push_back(i);
        }
    }

    timer.print();
}

//=========================================================================================

mutex m;

void generate_things(vector<int>& v)
{
    random_device rd;
    mt19937 mersenne(rd());
    uniform_int_distribution<int> urd(0, 1000);

    lock_guard lg(m);

    generate(v.begin(), v.end(), [&]()
        {
            return urd(mersenne);
        });
}

void pop_thing(vector<int>& v)
{
    lock_guard lg(m);

    cout << *max_element(v.begin(), v.end()) << endl;
}

void task_3()
{
    vector<int> v(100);
    size_t count = 100;

    while (count--)
    {
        thread owner([&]()
            {
                generate_things(v);
            });
        thread thief([&]()
            {
                pop_thing(v);
            });

        owner.join();
        thief.join();
    }
}

int main()
{
    task_1();
    task_2();
    task_3();

    return 0;
}



