import sys
from pulp import LpVariable, LpProblem, LpMinimize, LpStatus, PULP_CBC_CMD

def run(*argv):
    # print("len", len(argv))
    if len(argv) != 12:
        print("Usage: python solve_lp.py <A1 - A2> <B1 - B2> <C1 - C2> <N> <B> <D1 - D2> <i_low> <i_high> <j_low> <j_high> <k_low> <k_high>")
        sys.exit(1)
    print(argv)
    # Parse command line arguments
    c = [int(argv[0]), int(argv[1]), int(argv[2]), int(argv[3]), int(argv[4]), int(argv[5])]

    # Create the PuLP problem
    problem = LpProblem("Integer_Linear_Programming_Problem", LpMinimize)
    problem1 = LpProblem("Integer_Linear_Programming_Problem1", LpMinimize)

    # Define decision variables
    x1 = LpVariable("x1", lowBound=int(argv[6]), upBound=int(argv[7]), cat='Integer')
    x2 = LpVariable("x2", lowBound=int(argv[8]), upBound=int(argv[9]), cat='Integer')
    x3 = LpVariable("x3", lowBound=int(argv[10]), upBound=int(argv[11]), cat='Integer')
    x4 = LpVariable("x4", lowBound=None, upBound=None, cat='Integer')
    x5 = LpVariable("x5", lowBound=1, upBound=1, cat='Integer')

    # Add the objective function to the problem
    problem += 0, "Objective Function"
    problem1 += 0, "Objective Function"

    # Add constraints to the problem
    # problem += -int(argv[0])*x1 - int(argv[1])*x2 - int(argv[2])*x3 - int(argv[3])*x4 - int(argv[4])*x5 <= 0, "Constraint"
    problem += int(argv[0])*x1 + int(argv[1])*x2 + int(argv[2])*x3  + int(argv[5])*x5 <= 0, "Constraint" # a - b < 0
    problem += int(argv[0])*x1 + int(argv[1])*x2 + int(argv[2])*x3  + int(argv[5])*x5 - int(argv[3] * argv[4])*x4 +int(argv[4] * argv[4]) >= 1, "Constraint1" # a-b + kNB +B^2 > 0
    problem += int(argv[0])*x1 + int(argv[1])*x2 + int(argv[2])*x3  + int(argv[5])*x5 - int(argv[3] * argv[4])*x4 -int(argv[4] * argv[4]) <= -1, "Constraint2" # a-b + kNB -B^2 < 0
    problem += x4 <= -1, "Constraint3" # k < 0
    problem1 += int(argv[0])*x1 + int(argv[1])*x2 + int(argv[2])*x3  + int(argv[5])*x5 >= 1, "Constraint" # a - b > 0
    problem1 += int(argv[0])*x1 + int(argv[1])*x2 + int(argv[2])*x3  + int(argv[5])*x5 - int(argv[3] * argv[4])*x4 -int(argv[4] * argv[4]) <= -1, "Constraint1" # a-b + kNB -B^2 < 0
    problem1 += int(argv[0])*x1 + int(argv[1])*x2 + int(argv[2])*x3  + int(argv[5])*x5 - int(argv[3] * argv[4])*x4 +int(argv[4] * argv[4]) >= 1, "Constraint2" # a-b + kNB +B^2 > 0
    problem1 += x4 >= 0, "Constraint3" # k > 0
    # problem += -int(argv[0])*x1 - int(argv[1])*x2 - int(argv[2])*x3 - int(argv[3] * argv[4])*x4 - int(argv[5])*x5 >= -argv[4]+1, "Constraint1"

    # Solve the problem
    problem.solve(PULP_CBC_CMD(msg = False))   
    problem1.solve(PULP_CBC_CMD(msg = False))   

    # B = argv[4]
    # print("value:")
    # print(problem.objective.value())
    # print("status: ")
    # print(problem.status)
    # print("status1: ")
    # print(problem1.status)
    # Check if the problem is successfully solved
    if problem.status == 1 or problem1.status == 1:
        print("Optimal solution found!")
        # for variable in problem.variables():
        #     print(variable.name, "=", variable.value())
        for variable in problem1.variables():
            print(variable.name, "=", variable.value())
        return 1
    else:
        print("Solver failed to find a solution where the objective function is not 0.")
        return 0


if __name__ == "__main__":
    run(0,0,0,2,1,1,0,29,0,29,0,-1)
    sys.exit(1)
    if len(sys.argv) != 12:
        print("Usage: python solve_lp.py <A1 - A2> <B1 - B2> <C1 - C2> <N*B> <D1 - D2> <i_low> <i_high> <j_low> <j_high> <k_low> <k_high>")
        sys.exit(1)

    # Parse command line arguments
    c = [int(sys.argv[1]), int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4]), int(sys.argv[5])]

    # Create the PuLP problem
    problem = LpProblem("Integer Linear Programming Problem", LpMinimize)

    # Define decision variables
    x1 = LpVariable("x1", lowBound=int(sys.argv[6]), upBound=int(sys.argv[7]), cat='Integer')
    x2 = LpVariable("x2", lowBound=int(sys.argv[8]), upBound=int(sys.argv[9]), cat='Integer')
    x3 = LpVariable("x3", lowBound=int(sys.argv[10]), upBound=int(sys.argv[11]), cat='Integer')
    x4 = LpVariable("x4", lowBound=None, upBound=None, cat='Integer')
    x5 = LpVariable("x5", lowBound=1, upBound=1, cat='Integer')

    # Add the objective function to the problem
    problem += c[0]*x1 + c[1]*x2 + c[2]*x3 + c[3]*x4 + c[4]*x5, "Objective Function"

    # Add constraints to the problem
    problem += -int(sys.argv[1])*x1 - int(sys.argv[2])*x2 - int(sys.argv[3])*x3 - int(sys.argv[4])*x4 - int(sys.argv[5])*x5 <= 0, "Constraint"

    # Solve the problem
    problem.solve()

    # Check if the problem is successfully solved
    if problem.status == 1 and problem.objective.value() == 0:
        print("Optimal solution found:")
        for variable in problem.variables():
            print(variable.name, "=", variable.value())
    else:
        print("Solver failed to find a solution where the objective function is not 0.")
