from elasticsearch import Elasticsearch
from elasticsearch.exceptions import ConnectionError

# Configuration
API_ENDPOINT = "https://localhost:9200"
API_ID = "gV0VQ5MBzo9Pjs0TZi67"
API_KEY = "TXxDwIfsRKSyu_X8JUSP_A"
INDEX_NAME = "robot_logs"  # Specify the index name

def query_robot_logs():
    try:
        # Initialize the Elasticsearch client with API key authentication
        es = Elasticsearch(
            [API_ENDPOINT],
            api_key=(API_ID, API_KEY),
            verify_certs=False,  # Set to True if you have SSL certificates configured properly
            ssl_show_warn=False  # Suppresses warnings for self-signed certificates
        )

        # Query payload: Adjust as needed to fit your requirements
        query_payload = {
            "query": {
                "match_all": {}  # Retrieves all documents; customize for specific queries
            }
        }

        # Execute the query
        response = es.search(index=INDEX_NAME, body=query_payload)

        # Parse and display results
        print("Query Results:")
        for hit in response["hits"]["hits"]:
            print(hit["_source"])  # Print the document source

    except ConnectionError as e:
        print(f"Connection Error: {e}")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    query_robot_logs()
