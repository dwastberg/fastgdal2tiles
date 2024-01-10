//
// Created by Dag Wästberg on 2024-01-09.
//
#include "BS_thread_pool.hpp"

#include <iostream>
#include <thread>

void pool_pool(std::string pool_name)
{
    auto thread_nr_opt = BS::this_thread::get_index();
    auto thread_nr = thread_nr_opt.value();
    std::string out_str = pool_name + " " + std::to_string(thread_nr);
    std::cout << out_str <<std::endl;
}

void pool_info(std::string pool_name, BS::thread_pool &write_pool)
{
    auto thread_nr_opt = BS::this_thread::get_index();
    auto thread_nr = thread_nr_opt.value();
    std::string out_str = pool_name + " " + std::to_string(thread_nr);
    std::cout << out_str <<std::endl;

    write_pool.detach_task([](){pool_pool("write pool");});

}


int main()
{
    std::cout << "Hello World!" << std::endl;
    constexpr float render_pool_ratio = 0.5;
    size_t thread_count = std::thread::hardware_concurrency();
    size_t render_pool_size = (size_t) round(thread_count*render_pool_ratio);
    size_t write_pool_size = thread_count - render_pool_size;

    BS::thread_pool pool1(render_pool_size);
    BS::thread_pool pool2(write_pool_size);

    for (size_t i = 0; i<100 ; i++)
    {
        pool1.detach_task([&pool2](){ pool_info("Pool 1", pool2);});

    }
    pool1.wait();
    pool2.wait();

    std::cout << "total threads " << std::to_string(thread_count) << std::endl;


}