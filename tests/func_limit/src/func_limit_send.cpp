/******************************************************************************
 * The MIT Licence
 *
 * Copyright (c) 2020 Airbus Operations S.A.S
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *****************************************************************************/

/************
 * Includes *
 ************/

#include "test_context.h"
#include "sync_entity.h"

/***********
 * Defines *
 ***********/

#define TEST_ENTITY_SRC_ID 1
#define TEST_ENTITY_DST_ID 2

#define TEST_CONTEXT_SYNC_MAIN TestSend(); TestWait();
#define TEST_CONTEXT_SYNC_TESTER TestWait(); TestSend();

#define TEST_CONTEXT_SYNC TEST_CONTEXT_SYNC_MAIN

/********
 * Test *
 ********/

std::string config_path = "../config";

/******************************************************************************
This application is the sender application for the high load test
******************************************************************************/

class StreamContext : public TestContext {};

/******************************************************************************
This sequence sends at first frame per frame and then all frames at once
The preparation is always performed frame per frame
******************************************************************************/
TEST_P(StreamContext, LimitOneByOne)
{
    //ed247_set_log_level(ED247_LOG_LEVEL_ERROR);
    ed247_stream_list_t streams;
    ed247_stream_t stream;
    const ed247_stream_info_t* stream_info;
    size_t size;
    
    // Synchro at startup
    std::cout << "Startup" << std::endl;
    TEST_CONTEXT_SYNC
    
    ASSERT_EQ(ed247_component_get_streams(_context, &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_size(streams, &size), ED247_STATUS_SUCCESS);
    std::cout << "Stream number [" << size << "]" << std::endl;
    
    uint64_t start = synchro::get_time_us();
    for (uint32_t i = 0; i < size; i++)
    {
        ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);
        // std::cout << "Process Stream [" << stream_info->name << "]" << std::endl;
        uint32_t content = (uint32_t)stream_info->uid;
        ASSERT_EQ(ed247_stream_push_sample(stream, &content, sizeof(content), NULL, NULL), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
        
        TEST_CONTEXT_SYNC
    }
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    
    uint64_t end = synchro::get_time_us();
    std::cout << "Sending time (1 stream by 1 call) [" << (end-start)/1000 << "] ms" << std::endl;

    TEST_CONTEXT_SYNC

    // Unload
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
}

TEST_P(StreamContext, LimitAllInOne)
{
    //ed247_set_log_level(ED247_LOG_LEVEL_ERROR);
    ed247_stream_list_t streams;
    ed247_stream_t stream;
    const ed247_stream_info_t* stream_info;
    size_t size;
    
    // Synchro at startup
    std::cout << "Startup" << std::endl;
    TEST_CONTEXT_SYNC
    
    ASSERT_EQ(ed247_component_get_streams(_context, &streams), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_size(streams, &size), ED247_STATUS_SUCCESS);
    std::cout << "Stream number [" << size << "]" << std::endl;
    
    uint64_t start = synchro::get_time_us();
    for (uint32_t i = 0; i < size; i++)
    {
        ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
        ASSERT_EQ(ed247_stream_get_info(stream, &stream_info), ED247_STATUS_SUCCESS);
        // std::cout << "Process Stream [" << stream_info->name << "]" << std::endl;
        uint32_t content = (uint32_t)stream_info->uid;
        ASSERT_EQ(ed247_stream_push_sample(stream, &content, sizeof(content), NULL, NULL), ED247_STATUS_SUCCESS);
    }
    ASSERT_EQ(ed247_send_pushed_samples(_context), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_stream_list_next(streams, &stream), ED247_STATUS_SUCCESS);
    
    uint64_t end = synchro::get_time_us();
    std::cout << "Sending time (all streams by 1 call) [" << (end-start)/1000 << "] ms" << std::endl;
    TEST_CONTEXT_SYNC

    // Unload
    ASSERT_EQ(ed247_stream_list_free(streams), ED247_STATUS_SUCCESS);
}

std::vector<TestParams> stream_files;

INSTANTIATE_TEST_CASE_P(FT_LIMIT, StreamContext,
    ::testing::ValuesIn(stream_files));

/*************
 * Functions *
 *************/

int main(int argc, char **argv)
{
    if(argc >=1)
        config_path = argv[1];
    else
        config_path = "../config";

    std::cout << "Configuration path: " << config_path << std::endl;

    stream_files.push_back({TEST_ENTITY_SRC_ID, TEST_ENTITY_DST_ID, config_path+"/ecic_func_limit_send.xml"});

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}