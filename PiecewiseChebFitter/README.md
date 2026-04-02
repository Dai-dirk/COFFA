# Piecewise Chebyshev Polynomial Fitter (PWCPF)

This is a Piecewise Chebyshev Polynomial Fitter (**PWCPF**) for performing piecewise Chebyshev polynomial fitting on a given function. The tool can automatically determine optimal breakpoints and the order of Chebyshev polynomials for each segment to achieve the best fitting results.

## Key Features

- Automatically determines optimal breakpoints
- Automatically optimizes the Chebyshev polynomial order for each segment
- Supports multiple error evaluation methods (MSE, MAE)
- Provides two order allocation methods: genetic algorithm and exhaustive search
- Visualizes fitting results
- Supports saving and exporting results

## Installing Dependencies

```bash
pip install -r requirements.txt
```

## How to use

```bash
python run_func.py function_name (e.g., softplus)
```

### Prepare a configuration file (default.json): See config/softplus/softplus.json as an example
```json
{
    "alpha": 100000000,
    "beta": 0,
    "coef_norm": true,
    "max_order_sum": 100,
    "sample_count": 35000,
    "piecewise_threshold": 0.001,
    "error_evaluation": "mse",
    "order_distribute_method": "ga",
    "ga_config": {
        "population_size": 16,
        "generations": 10,
        "lambda_": 16,
        "mutation_rate": 0.2,
        "crossover_rate": 0.7,
        "multiprocessing": true,
        "pool_size": 16
    },
    "verbose": true,
    "Parity": 0,
    "max_segments": 6,
    "name": Your_required_function_name,
    "int_width": 0,
    "frac_width": 32,
    "max_order": 5,
    "domain": [
        -8,
        8
    ]
}
```

### Key Configuration Explanation

- **`name`**: The name of the function
- **`domain`**: The range of the domain
- **`target_function`**: The target function expression
- **`sample_count`**: The number of sample points
- **`max_segments`**: The maximum number of segments
- **`max_order`**: The maximum polynomial order for each segment
- **`max_order_sum`**: The upper limit for the total polynomial order
- **`piecewise_threshold`**: The minimum distance between breakpoints
- **`error_evaluation`**: The error evaluation method ("mse" or "mae")
- **`alpha`**: The error weight
- **`beta`**: The polynomial order weight
- **`gamma`**: The number of segments weight
- **`order_distribute_method`**: The polynomial order distribution method ("ga" for Genetic Algorithm, "traversal" for manual traversal)
- **`ga_config`**: The configuration for the genetic algorithm, especially the "pool_size" parameter indicates how many CPU cores are used for evaluation per generation
- **`Parity`**: The parity of the target function
- **`verbose`**: Whether to display detailed output


### Output: See result/softplus and fig/softplus as an example

The fitting results will include:
- Number of segments
- Locations of breakpoints
- Chebyshev polynomial order for each segment
- Total polynomial order
- Fitting error
- Target function values
- Chebyshev coefficients
- Polynomial coefficients

Additionally, visualizations of the fitting results (figures) will be generated.