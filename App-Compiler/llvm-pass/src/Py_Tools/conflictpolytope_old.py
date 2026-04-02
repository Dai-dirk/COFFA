import sys
from pulp import LpVariable, LpProblem, LpMinimize, LpStatus, PULP_CBC_CMD

def run(*argv):
    # print("len", len(argv))
    if len(argv) != 12:
        print("Usage: python solve_lp.py <A1 - A2> <B1 - B2> <C1 - C2> <N> <B> <D1 - D2> <i_low> <i_high> <j_low> <j_high> <k_low> <k_high>")
        sys.exit(1)
    # print(argv)
    # Parse command line arguments
    c = [float(argv[0]), float(argv[1]), float(argv[2]), float(argv[3]), float(argv[4]), float(argv[5])]

    # Create the PuLP problem
    problem = LpProblem("Integer_Linear_Programming_Problem", LpMinimize)

    # Define decision variables
    x1 = LpVariable("x1", lowBound=float(argv[6]), upBound=float(argv[7]), cat='Integer')
    x2 = LpVariable("x2", lowBound=float(argv[8]), upBound=float(argv[9]), cat='Integer')
    x3 = LpVariable("x3", lowBound=float(argv[10]), upBound=float(argv[11]), cat='Integer')
    x4 = LpVariable("x4", lowBound=None, upBound=None, cat='Integer')
    x5 = LpVariable("x5", lowBound=1, upBound=1, cat='Integer')

    # Add the objective function to the problem
    problem += c[0]*x1 + c[1]*x2 + c[2]*x3 + (c[3]*c[4])*x4 + c[5]*x5, "Objective Function"

    # Add constraints to the problem
    # problem += -float(argv[0])*x1 - float(argv[1])*x2 - float(argv[2])*x3 - float(argv[3])*x4 - float(argv[4])*x5 <= 0, "Constraint"
    problem += -float(argv[0])*x1 - float(argv[1])*x2 - float(argv[2])*x3 - float(argv[3] * argv[4])*x4 - float(argv[5])*x5 <= 0, "Constraint"
    problem += -float(argv[0])*x1 - float(argv[1])*x2 - float(argv[2])*x3 - float(argv[3] * argv[4])*x4 - float(argv[5])*x5 >= -argv[4]+1, "Constraint1"

    # Solve the problem
    problem.solve(PULP_CBC_CMD(msg = False))   

    B = argv[4]
    # print("value:")
    # print(problem.objective.value())
    # Check if the problem is successfully solved
    if problem.status == 1:
        # print("Optimal solution found:")
        # for variable in problem.variables():
        #     print(variable.name, "=", variable.value())
        return 1
    else:
        # print("Solver failed to find a solution where the objective function is not 0.")
        return 0


if __name__ == "__main__":
    run(0,0,0,1,1,-1,0,500,0,0,0,0)
    sys.exit(1)
    if len(sys.argv) != 12:
        print("Usage: python solve_lp.py <A1 - A2> <B1 - B2> <C1 - C2> <N*B> <D1 - D2> <i_low> <i_high> <j_low> <j_high> <k_low> <k_high>")
        sys.exit(1)

    # Parse command line arguments
    c = [float(sys.argv[1]), float(sys.argv[2]), float(sys.argv[3]), float(sys.argv[4]), float(sys.argv[5])]

    # Create the PuLP problem
    problem = LpProblem("Integer Linear Programming Problem", LpMinimize)

    # Define decision variables
    x1 = LpVariable("x1", lowBound=float(sys.argv[6]), upBound=float(sys.argv[7]), cat='Integer')
    x2 = LpVariable("x2", lowBound=float(sys.argv[8]), upBound=float(sys.argv[9]), cat='Integer')
    x3 = LpVariable("x3", lowBound=float(sys.argv[10]), upBound=float(sys.argv[11]), cat='Integer')
    x4 = LpVariable("x4", lowBound=None, upBound=None, cat='Integer')
    x5 = LpVariable("x5", lowBound=1, upBound=1, cat='Integer')

    # Add the objective function to the problem
    problem += c[0]*x1 + c[1]*x2 + c[2]*x3 + c[3]*x4 + c[4]*x5, "Objective Function"

    # Add constraints to the problem
    problem += -float(sys.argv[1])*x1 - float(sys.argv[2])*x2 - float(sys.argv[3])*x3 - float(sys.argv[4])*x4 - float(sys.argv[5])*x5 <= 0, "Constraint"

    # Solve the problem
    problem.solve()

    # Check if the problem is successfully solved
    if problem.status == 1 and problem.objective.value() == 0:
        print("Optimal solution found:")
        for variable in problem.variables():
            print(variable.name, "=", variable.value())
    else:
        print("Solver failed to find a solution where the objective function is not 0.")
