from elasticsearch import Elasticsearch
from elasticsearch.exceptions import ConnectionError
import matplotlib.pyplot as plt
import math

# Configuration
API_ENDPOINT = "https://localhost:9200"
API_ID = "gV0VQ5MBzo9Pjs0TZi67"
API_KEY = "TXxDwIfsRKSyu_X8JUSP_A"
INDEX_NAME = "robot_logs"  # Specify the index name

def query_and_plot_robot_logs():
    try:
        # Initialize the Elasticsearch client with API key authentication
        es = Elasticsearch(
            [API_ENDPOINT],
            api_key=(API_ID, API_KEY),
            verify_certs=False,  # Set to True if you have SSL certificates configured properly
            ssl_show_warn=False  # Suppresses warnings for self-signed certificates
        )

        # Query payload to retrieve all robot logs
        query_payload = {
            "size": 1000,  # Fetch 1000 documents
            "query": {
                "match_all": {}
            }
        }

        # Execute the query
        response = es.search(index=INDEX_NAME, body=query_payload)

        # Parse results
        robots_positions = {}
        for hit in response["hits"]["hits"]:
            doc = hit["_source"]

            # Extract robot_id and position
            robot_id = None
            x, y = None, None
            try:
                if "message" in doc:
                    message = doc["message"]
                    if "Robot ID:" in message:
                        robot_id = message.split("Robot ID: ")[1].split(" ")[0]
                    if "Position:" in message:
                        position_str = message.split("Position: (")[1].split(")")[0]
                        x, y = map(float, position_str.split(","))
            except Exception as e:
                print(f"Error parsing document: {e}")

            if robot_id:
                # Track initial and final positions
                if robot_id not in robots_positions:
                    robots_positions[robot_id] = {"initial": (x, y), "final": (x, y)}
                else:
                    robots_positions[robot_id]["final"] = (x, y)

        # Calculate distances covered
        robots_distances = {}
        for robot_id, positions in robots_positions.items():
            initial = positions["initial"]
            final = positions["final"]
            if initial and final:
                distance = math.sqrt((final[0] - initial[0]) ** 2 + (final[1] - initial[1]) ** 2)
                robots_distances[robot_id] = distance

        # Prepare data for plotting
        robot_ids = list(robots_distances.keys())
        distances = list(robots_distances.values())

        # Handle case where no robots have data
        if not robot_ids:
            print("No robots with valid position data found.")
            return

        # Dynamically adjust the figure size based on the number of robots
        num_robots = len(robot_ids)
        plt.figure(figsize=(max(10, num_robots * 0.5), 6))  # Adjust width based on the number of robots

        # Plotting
        bars = plt.bar(robot_ids, distances, color="skyblue", edgecolor="black")
        plt.xlabel("Robot ID", fontsize=12)
        plt.ylabel("Total Distance Covered (units)", fontsize=12)
        plt.title("Total Ground Covered by Robots", fontsize=14)
        plt.xticks(rotation=45, fontsize=10)
        plt.yticks(fontsize=10)
        plt.tight_layout()

        # Add value annotations to the bars
        for bar in bars:
            height = bar.get_height()
            if height > 0:
                plt.text(
                    bar.get_x() + bar.get_width() / 2,
                    height,
                    f"{height:.2f}",
                    ha="center",
                    va="bottom",
                    fontsize=10
                )

        # Display the plot
        plt.show()

    except ConnectionError as e:
        print(f"Connection Error: {e}")
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    query_and_plot_robot_logs()
