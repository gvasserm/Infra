import json
import numpy as np

def deserialize_cv_mat(json_path):
    with open(json_path, 'r') as file:
        data = json.load(file)['value0']
    
    # Extract the necessary information
    dims = data["dims"]
    rows = data["rows"]
    cols = data["cols"]
    # For simplicity, assuming type corresponds to a float, adjust according to actual data type
    mat_type = np.float32
    
    # Check if 'data' field is present (for continuous matrices)
    if "data" in data:
        # Assuming the data is stored in a flat list, reshape it to match the original dimensions
        array_data = np.array(data["data"], dtype=mat_type).reshape((rows, cols))
    else:
        # Handle non-continuous or differently structured data as needed
        array_data = None  # Placeholder for alternative handling

    return array_data

# Example usage
json_path = '/home/gvasserm/dev/Infra/DEVL/Cloud_1.dump'
mat = deserialize_cv_mat(json_path)

print(mat)