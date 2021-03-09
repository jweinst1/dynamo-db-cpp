#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>
#include <mutex>
#include <aws/core/Aws.h>
#include <aws/core/utils/Outcome.h> 
#include <aws/dynamodb/DynamoDBClient.h>
#include <aws/dynamodb/model/AttributeDefinition.h>
#include <aws/dynamodb/model/PutItemRequest.h>
#include <aws/dynamodb/model/PutItemResult.h>
#include <aws/dynamodb/model/CreateTableRequest.h>
#include <aws/dynamodb/model/KeySchemaElement.h>
#include <aws/dynamodb/model/ProvisionedThroughput.h>
#include <aws/dynamodb/model/ScalarAttributeType.h>
#include <aws/dynamodb/model/DeleteTableRequest.h>


struct ScopedAwsSDK {

	ScopedAwsSDK() {
		Aws::InitAPI(opts);
	}

	~ScopedAwsSDK() {
		Aws::ShutdownAPI(opts);
	}

	Aws::SDKOptions opts;
};


int main(int argc, char const *argv[])
{
	ScopedAwsSDK sdkScoped;
	return 0;
}