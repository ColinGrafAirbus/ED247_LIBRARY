/******************************************************************************
 * The MIT Licence
 *
 * Copyright (c) 2021 Airbus Operations S.A.S
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

#include "unitary_test.h"


std::string config_path = "../config";

class UtApiMisc : public ::testing::Test {};

/******************************************************************************
Sequence checking the information getter
******************************************************************************/
TEST(UtApiMisc, InfoGetter)
{
    ed247_context_t context = nullptr;
    const ed247_component_info_t* infos = nullptr;
    ed247_component_info_t expected_info_values = {
        "", // Version
        ED247_COMPONENT_TYPE_VIRTUAL, // Type
        "ComponentWithoutOptions", // Name
        "", // Comment
        ED247_STANDARD_ED247A, // Standard Revision
        0,
        { // File Producer
            "", // Identifier
            "" // Comment
        }
    };
    // First 2 calls are not functional, just check they do not make any crash in application
    ASSERT_EQ(ed247_component_get_info(NULL, &infos), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_component_get_info(context, NULL), ED247_STATUS_FAILURE);

    std::string filepath = config_path+"/ecic_unit_api_misc_mini.xml";
    ASSERT_EQ(ed247_load_file(filepath.c_str(), &context), ED247_STATUS_SUCCESS);
    malloc_count_start();
    ASSERT_EQ(ed247_component_get_info(NULL, &infos), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_component_get_info(context, NULL), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_component_get_info(context, &infos), ED247_STATUS_SUCCESS);
    ASSERT_EQ(malloc_count_stop(), 0);
    
    // Check content information
    ASSERT_EQ(infos->component_type, expected_info_values.component_type);
    ASSERT_TRUE(infos->name != NULL && strcmp(infos->name, expected_info_values.name) == 0);
    ASSERT_TRUE(infos->comment != NULL && strcmp(infos->comment, expected_info_values.comment) == 0);
    ASSERT_EQ(infos->standard_revision, expected_info_values.standard_revision);
    ASSERT_TRUE(infos->component_version != NULL && strcmp(infos->component_version, expected_info_values.component_version) == 0);
    ASSERT_TRUE(infos->file_producer.identifier != NULL && strcmp(infos->file_producer.identifier, expected_info_values.file_producer.identifier) == 0 );
    ASSERT_TRUE(infos->file_producer.comment != NULL && strcmp(infos->file_producer.comment, expected_info_values.file_producer.comment) == 0);
    
    // Load a second configuration with optional fields filled
    expected_info_values = {
        "X.Y", // Version
        ED247_COMPONENT_TYPE_BRIDGE, // Type
        "ComponentWithAllOptions", // Name
        "Toutes les options du header sont renseignées", // Comment
        ED247_STANDARD_ED247A, // Standard Revision
        0,
        { // File Producer
            "User", // Identifier
            "COMMENT" // Comment
        }
    };
    
    filepath = config_path+"/ecic_unit_api_misc.xml";
    ASSERT_EQ(ed247_load_file(filepath.c_str(), &context), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_component_get_info(context, &infos), ED247_STATUS_SUCCESS);
    ASSERT_EQ(ed247_component_get_info(NULL, &infos), ED247_STATUS_FAILURE);
    ASSERT_EQ(ed247_component_get_info(context, NULL), ED247_STATUS_FAILURE);
    
    // Check content information
    ASSERT_EQ(infos->component_type, expected_info_values.component_type);
    ASSERT_FALSE(strcmp(infos->name, expected_info_values.name));
    ASSERT_FALSE(strcmp(infos->comment, expected_info_values.comment));
    ASSERT_EQ(infos->standard_revision, expected_info_values.standard_revision);
    ASSERT_FALSE(strcmp(infos->component_version, expected_info_values.component_version));
    ASSERT_FALSE(strcmp(infos->file_producer.identifier, expected_info_values.file_producer.identifier));
    ASSERT_FALSE(strcmp(infos->file_producer.comment, expected_info_values.file_producer.comment));

    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE__INVALID), (size_t)0);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_INT8), (size_t)1);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_INT16), (size_t)2);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_INT32), (size_t)4);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_INT64), (size_t)8);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_UINT8), (size_t)1);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_UINT16), (size_t)2);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_UINT32), (size_t)4);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_UINT64), (size_t)8);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_FLOAT32), (size_t)4);
    ASSERT_EQ(ed247_nad_type_size(ED247_NAD_TYPE_FLOAT64), (size_t)8);
    
    ASSERT_EQ(ed247_unload(context), ED247_STATUS_SUCCESS);
}

/******************************************************************************
Check the library identification routines
******************************************************************************/
TEST(UtApiMisc, IdentificationGetters)
{
    const char* value = NULL;
    value = ed247_get_implementation_name();
    ASSERT_STREQ(value, "ED247_LIBRARY");
    
    value = ed247_get_implementation_version();
    ASSERT_STREQ(value, TEST_PRODUCT_VERSION);
}

int main(int argc, char **argv)
{
    if(argc >=1)
        config_path = argv[1];
    else
        config_path = "../config";

    SAY("Configuration path: " << config_path);

    ::testing::InitGoogleTest(&argc, argv);
    // ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
