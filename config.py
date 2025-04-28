import math

# Parsing topo.txt
def parse_topo_file(file_path):
    with open(file_path, 'r') as f:
        lines = [line.strip() for line in f.readlines() if line.strip()]
    
    config = {}
    
    # Parse the header lines
    for line in lines[:1]:
        if "Number of client nodes" in line:
            config['num_clients'] = int(line.split('=')[1].strip())
    
    # Parsing client task sizes
    client_tasks = {}
    for line in lines[1:]:
        if "Client" in line and ":" in line:
            parts = line.split(":")
            client_id = int(parts[0].split()[1].strip()) - 1  # Converting to 0-based indexing
            task_sizes = parts[1].strip()                      # Keeping as string
            client_tasks[client_id] = task_sizes
    
    config['client_tasks'] = client_tasks
    
    return config

def generate_task_size_condition(client_tasks, num_clients):
    result = []
    defined_clients = len(client_tasks)
    for i in range(num_clients):
        if i in client_tasks:
            task_sizes = f'"{client_tasks[i]}"'
            if i == 0:
                result.append(f"index==0 ? {task_sizes} : \n")
            else:
                result.append(f"                           (index=={i} ? {task_sizes} : \n")
    
    # Adding the default case
    result.append("                           \"10\"")
    
    # Adding closing parentheses - one for each client condition except the first one
    defined_clients_after_first = sum(1 for k in client_tasks.keys() if k > 0)
    result.append(")" * defined_clients_after_first)
    result.append(";")
    
    return "".join(result)

def generate_network_ned(config, output_file):  
    # Generate the conditional task size assignment with correct parentheses
    task_size_condition = generate_task_size_condition(config['client_tasks'], config['num_clients'])
    
    # Calculate the number of finger table connections needed for O(log N) routing
    log_n = math.ceil(math.log2(config['num_clients']))
    
    ned_content = f"""// B22CS081_B22CS030 P2P Network with CHORD-like routing

simple ClientNode
{{
    parameters:
        int id;
        int numClients;
        string taskSizes;
    gates:
        // For communication with other clients (both ring and finger table connections)
        input in[numClients];
        output out[numClients];
}}

network P2PAssignment
{{
    parameters:
        int numClients = default({config['num_clients']});
    
    types:
        channel DelayChannel extends ned.DelayChannel {{
            delay = 100ms;
        }}
    
    submodules:
        client[numClients]: ClientNode {{
            parameters:
                id = index;
                numClients = parent.numClients;
                taskSizes = {task_size_condition}
        }}

    connections allowunconnected:
        // Create ring topology connections
        for i=0..numClients-1 {{
            client[i].out[(i+1)%numClients] --> DelayChannel --> client[(i+1)%numClients].in[i];
            client[i].out[(i-1+numClients)%numClients] --> DelayChannel --> client[(i-1+numClients)%numClients].in[i];
        }}
        
        // Create CHORD-like finger table connections
        // Each node i has connections to nodes (i+2^j) % numClients for j = 0,1,2,...,log2(numClients)-1
        // Using a fixed value for the maximum j since log2() is not available in NED language
        for i=0..numClients-1, for j=0..{log_n-1} {{
            client[i].out[(i+(1<<j))%numClients] --> DelayChannel --> client[(i+(1<<j))%numClients].in[i];
        }}
}}
"""
    with open(output_file, 'w') as f:
        f.write(ned_content)
    print(f"Successfully generated {output_file}")

def main():
    topo_file = "topo.txt"
    network_ned_file = "Network.ned"
    
    try:
        config = parse_topo_file(topo_file)
        generate_network_ned(config, network_ned_file)
    except Exception as e:
        print(f"Error: {e}")
        return 1
    
    return 0

if __name__ == "__main__":
    exit(main())