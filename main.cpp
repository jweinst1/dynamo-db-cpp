#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>
#include <mutex>
#include <cstdlib>
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
#include <aws/dynamodb/model/ScanRequest.h>
#include <aws/dynamodb/model/ScanResult.h>


struct ScopedAwsSDK {

	ScopedAwsSDK() {
		Aws::InitAPI(opts);
	}

	~ScopedAwsSDK() {
		Aws::ShutdownAPI(opts);
	}

	Aws::SDKOptions opts;
};

class ScopedDynamoTable {
public:
	ScopedDynamoTable(const char* name, const char* keyName): _name(name),
	                                                          _keyName(keyName) 
	{
		Aws::Client::ClientConfiguration clientConfig;
		_client = Aws::MakeUnique<Aws::DynamoDB::DynamoDBClient>("Dynamo Alloc", clientConfig);
		Aws::DynamoDB::Model::CreateTableRequest req;
		Aws::DynamoDB::Model::AttributeDefinition hashKey;
		hashKey.SetAttributeName(_keyName);
		hashKey.SetAttributeType(Aws::DynamoDB::Model::ScalarAttributeType::S);
		req.AddAttributeDefinitions(hashKey);

		Aws::DynamoDB::Model::KeySchemaElement keyscelt;
		keyscelt.WithAttributeName(_keyName).WithKeyType(Aws::DynamoDB::Model::KeyType::HASH);
		req.AddKeySchema(keyscelt);

		Aws::DynamoDB::Model::ProvisionedThroughput thruput;
		thruput.WithReadCapacityUnits(5).WithWriteCapacityUnits(5);
		req.SetProvisionedThroughput(thruput);

		req.SetTableName(_name);

		const Aws::DynamoDB::Model::CreateTableOutcome& result = _client->CreateTable(req);
		if (result.IsSuccess()) {
		    std::cout << "Table: '" << result.GetResult().GetTableDescription().GetTableName() <<
		        "' created successfully\n";
		    for (int i = 0; i < 10; ++i)
		    {
		    	std::cout << "Waiting until table is ready....\n";
		        std::this_thread::sleep_for(std::chrono::seconds(1));
		    }
		} else {
		    std::cerr << "Failed to create table: " << result.GetError().GetMessage();
		    std::abort();
		}

	}

	bool putItem(const char* keyValue)
	{
		Aws::DynamoDB::Model::PutItemRequest pir;
		pir.SetTableName(_name);

		const Aws::String condExpression = "attribute_not_exists(" + _keyName + ")";
		pir.SetConditionExpression(condExpression);

		Aws::DynamoDB::Model::AttributeValue av;
		av.SetS(keyValue);
		pir.AddItem(_keyName, av);

		const auto result = _client->PutItem(pir);
		if (!result.IsSuccess()) {
		    return false;
		}
		return true;
	}

	bool printItems()
	{
		Aws::DynamoDB::Model::ScanRequest sreq;
		sreq.SetTableName(_name);

		const auto result = _client->Scan(sreq);
		if (result.IsSuccess()) {
			const auto scanResult = result.GetResult();
			std::cout << "Scanned a total of " << scanResult.GetScannedCount() << " items\n";
			const auto scanItems = scanResult.GetItems();
			for (auto sit = scanItems.begin(); sit != scanItems.end(); ++sit)
			{
				std::cout << "{" << _keyName << " : " << sit->at(_keyName).GetS()
				          << "}\n";
			}
			return true;
		} else {
			std::cerr << "Failed to perform scan on table " << _name;
		}
		return false;
	}

	~ScopedDynamoTable() {
		Aws::DynamoDB::Model::DeleteTableRequest dtr;
		dtr.SetTableName(_name);

		const auto result = _client->DeleteTable(dtr);
		if (result.IsSuccess()) {
		    std::cout << "Table \"" << result.GetResult().GetTableDescription().GetTableName() <<
		        "\" has been deleted\n";
		} else {
		    std::cerr << "Failed to delete the table: " << result.GetError().GetMessage();
		    std::abort();
		}

	}
private:
	Aws::String _name;
	Aws::String _keyName;
	Aws::UniquePtr<Aws::DynamoDB::DynamoDBClient> _client;
};

static const char* KEY_NAME = "event";

int main(int argc, char const *argv[])
{
	ScopedAwsSDK sdkScoped;
	{
		ScopedDynamoTable table("schedule", KEY_NAME);
		table.putItem("morning");
		table.putItem("lunch");
		table.putItem("basketball");
		table.printItems();
	}
	return 0;
}