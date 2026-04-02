import sys
from pulp import LpVariable, LpProblem, LpMinimize, LpStatus, PULP_CBC_CMD,lpSum

def run(*argv):
    # print("len", len(argv))
    if len(argv) != 17:
        print("Usage: python solve_lp.py <A1> <B1> <C1> <D1> <A2> <B2> <C2> <D2><N> <B> <i_low> <i_high> <j_low> <j_high> <k_low> <k_high> <distance>")#distance = node1 - node2
        sys.exit(1)
    # print(argv)
    # Parse command line arguments
    phi_1 = [int(argv[0]), int(argv[1]), int(argv[2]), int(argv[3])]
    phi_2 = [int(argv[4]), int(argv[5]), int(argv[6]), int(argv[7])]
    # print("argv:")
    # for variable in argv:
    #     print(variable)
    # print("phi_2:")
    # for variable in phi_2:
    #     print(variable)
    # scheduled-distance
    sche_dis = int(argv[16])

    # Create the PuLP problem
    problem = LpProblem("Integer_Linear_Programming_Problem", LpMinimize)

    # Define decision variables
    x1 = LpVariable("x1", lowBound=int(argv[10]), upBound=int(argv[11]), cat='Integer')
    x2 = LpVariable("x2", lowBound=int(argv[12]), upBound=int(argv[13]), cat='Integer')
    x3 = LpVariable("x3", lowBound=int(argv[14]), upBound=int(argv[15]), cat='Integer')
    # x4 = LpVariable("x4", lowBound=None, upBound=None, cat='Integer')
    x4 = LpVariable("x4", lowBound=1, upBound=1, cat='Integer') # the constant
    x5 = LpVariable("x5", lowBound=None, upBound=None, cat='Integer')# kN: k
    x6 = LpVariable("x6", lowBound=int(argv[10]), upBound=int(argv[11]), cat='Integer')
    x7 = LpVariable("x7", lowBound=int(argv[12]), upBound=int(argv[13]), cat='Integer')
    x8 = LpVariable("x8", lowBound=int(argv[14]), upBound=int(argv[15]), cat='Integer')

    # Add the objective function to the problem
    problem += 0, "Objective Function"
    
    # Define a new variable for the division result
    z1 = LpVariable("z1", lowBound=None, upBound=None, cat='Integer')
    z2 = LpVariable("z2", lowBound=None, upBound=None, cat='Integer')

    # [a/B] Ensure that z1/z2 is integer
    problem += z1 * int(argv[9]) <= phi_1[0]*x1 + phi_1[1]*x2 + phi_1[2]*x3 + phi_1[3]*x4
    problem += z2 * int(argv[9]) <= phi_2[0]*x6 + phi_2[1]*x7 + phi_2[2]*x8 + phi_2[3]*x4
    problem += (z1 + 1) * int(argv[9]) - (phi_1[0]*x1 + phi_1[1]*x2 + phi_1[2]*x3 + phi_1[3]*x4) >= 1
    problem += (z2 + 1) * int(argv[9]) - (phi_2[0]*x6 + phi_2[1]*x7 + phi_2[2]*x8 + phi_2[3]*x4) >= 1
    problem += (x1 + x2 * (int(argv[11]) - int(argv[10]) + 1) + x3 * (int(argv[13]) - int(argv[12]) + 1) * (int(argv[11]) - int(argv[10]) + 1))  - (x6 + x7 * (int(argv[11]) - int(argv[10]) + 1)  + x8 * (int(argv[13]) - int(argv[12]) + 1) * (int(argv[11]) - int(argv[10]) + 1)) >= sche_dis
    problem += (x1 + x2 * (int(argv[11]) - int(argv[10]) + 1) + x3 * (int(argv[13]) - int(argv[12]) + 1) * (int(argv[11]) - int(argv[10]) + 1))  - (x6 + x7 * (int(argv[11]) - int(argv[10]) + 1)  + x8 * (int(argv[13]) - int(argv[12]) + 1) * (int(argv[11]) - int(argv[10]) + 1)) <= sche_dis



    # Add constraints to represent the division operation
    problem += phi_1[0]*x1 + phi_1[1]*x2 + phi_1[2]*x3 + phi_1[3]*x4 - (phi_2[0]*x6 + phi_2[1]*x7 + phi_2[2]*x8 + phi_2[3]*x4) - int(argv[8] * argv[9])*x5 -int(argv[9]) <= -1, "Constraint1" # a-b - kNB -B < 0
    problem += phi_1[0]*x1 + phi_1[1]*x2 + phi_1[2]*x3 + phi_1[3]*x4 - (phi_2[0]*x6 + phi_2[1]*x7 + phi_2[2]*x8 + phi_2[3]*x4) - int(argv[8] * argv[9])*x5 +int(argv[9])  >= 1, "Constraint2" # a-b - kNB +B > 0
    # problem += (phi_1[0]*x1 + phi_1[1]*x2 + phi_1[2]*x3 + phi_1[3]*x4) - (phi_2[0]*x6 + phi_2[1]*x7 + phi_2[2]*x8 + phi_2[3]*x4) == int(argv[8])*x5, "Constraint3" # a-b - kNB +B^2 > 0
    problem += z1 - z2 == int(argv[8])*x5, "Constraint3" # a-b - kNB +B^2 > 0


    # Solve the problem
    problem.solve(PULP_CBC_CMD(msg = False))   

    # print("status: ")
    # print(problem.status)
    # for variable in problem.variables():
    #     print(variable.name, "=", variable.value())
    # return 1
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
    run(-1,0,0,2046,1,0,0,1,4,1024,0,2046,0,0,0,0,1)
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
