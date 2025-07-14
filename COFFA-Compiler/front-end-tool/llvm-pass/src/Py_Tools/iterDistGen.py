import sys
from pulp import LpVariable, LpProblem, LpMinimize, LpStatus, PULP_CBC_CMD,lpSum

def run(*argv):
    # print("len", len(argv))
    if len(argv) != 14:
        print("Usage: python solve_lp.py <A1> <B1> <C1> <D1> <A2> <B2> <C2> <D2> <i_low> <i_high> <j_low> <j_high> <k_low> <k_high>")#distance = node1 - node2
        sys.exit(1)
    print(argv)
    # Parse command line arguments
    phi_store = [int(argv[0]), int(argv[1]), int(argv[2]), int(argv[3])]
    phi_load = [int(argv[4]), int(argv[5]), int(argv[6]), int(argv[7])]
    # print("argv:")
    # for variable in argv:
    #     print(variable)
    # print("phi_load:")
    # for variable in phi_load:
    #     print(variable)
    # scheduled-distance
    # sche_dis = int(argv[16])

    # Create the PuLP problem
    problem = LpProblem("Integer_Linear_Programming_Problem", LpMinimize)

    # Define decision variables
    x1 = LpVariable("x1", lowBound=int(argv[8]), upBound=int(argv[9]), cat='Integer')
    x2 = LpVariable("x2", lowBound=int(argv[10]), upBound=int(argv[11]), cat='Integer')
    x3 = LpVariable("x3", lowBound=int(argv[12]), upBound=int(argv[13]), cat='Integer')
    # x4 = LpVariable("x4", lowBound=None, upBound=None, cat='Integer')
    x4 = LpVariable("x4", lowBound=1, upBound=1, cat='Integer') # the constant
    x5 = LpVariable("x5", lowBound=int(argv[8]), upBound=int(argv[9]), cat='Integer')
    x6 = LpVariable("x6", lowBound=int(argv[10]), upBound=int(argv[11]), cat='Integer')
    x7 = LpVariable("x7", lowBound=int(argv[12]), upBound=int(argv[13]), cat='Integer')

    # Add the objective function to the problem
    problem += (x5 + x6 * (int(argv[9]) - int(argv[8]) + 1)  + x7 * (int(argv[11]) - int(argv[10]) + 1) * (int(argv[9]) - int(argv[8]) + 1)) - (x1 + x2 * (int(argv[9]) - int(argv[8]) + 1) + x3 * (int(argv[11]) - int(argv[10]) + 1) * (int(argv[9]) - int(argv[8]) + 1)), "Objective Function"
    problem += (x5 + x6 * (int(argv[9]) - int(argv[8]) + 1)  + x7 * (int(argv[11]) - int(argv[10]) + 1) * (int(argv[9]) - int(argv[8]) + 1)) - (x1 + x2 * (int(argv[9]) - int(argv[8]) + 1) + x3 * (int(argv[11]) - int(argv[10]) + 1) * (int(argv[9]) - int(argv[8]) + 1)) >= 1, "Constraint1"
    # problem += (x5 + x6 * (int(argv[9]) - int(argv[8]) + 1)  + x7 * (int(argv[11]) - int(argv[10]) + 1) * (int(argv[9]) - int(argv[8]) + 1)) - (x1 + x2 * (int(argv[9]) - int(argv[8]) + 1) + x3 * (int(argv[11]) - int(argv[10]) + 1) * (int(argv[9]) - int(argv[8]) + 1)) >= 4, "Constraint2"
    problem += (phi_store[0]*x1 + phi_store[1]*x2 + phi_store[2]*x3 + phi_store[3]*x4) == (phi_load[0]*x5 + phi_load[1]*x6 + phi_load[2]*x7 + phi_load[3]*x4), "Constraint2" # phi_store = phi_load

    # Check if the problem is successfully solved
    solutions = []
    # Solve the problem
    problem.solve(PULP_CBC_CMD(msg = False))   
    if problem.status == 1:
        print("solution found:")
        print("Optimal solution found:")
        for variable in problem.variables():
            print(variable.name, "=", variable.value())
        print("Objective value: ", problem.objective.value())
        solutions.append(problem.objective.value())
        # return 1
    else:
        solutions.append(0)
        print("Solver failed to find a solution.")
        # return 0
    solutions.sort()
    # for variable in solutions:
    #     print(variable)
    # print("Objective value: ", solutions[0])
    result = int(solutions[0])
    return result


if __name__ == "__main__":
    # run(1,16,0,17,1,16,0,0,0,7,0,7,0,0)
    # run(4,0,0,0,1,0,0,0,0,15,0,0,0,0)
    run(1,0,0,1,-1,0,0,14,0,14,0,0,0,0)
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
