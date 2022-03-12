#	Le Uyen Nguyen
#	100 171 8086
#	04/01/2021
#	Windows 10 - Python 3.9.2 - IDLE Shell

# This file includes the Extra Credit_part B
# The operators added include unary subtraction,
# modulo division, and exponentiation.

import os

# Function is_num() takes input n, return a boolean value,
# which is true if n is an integer; else, false.
def is_num(n):
    try:
        int(n)
        return True
    except ValueError:
        return False

def main():
    # Get the file path
    directory = os.path.dirname(os.path.abspath(__file__))
    file_path = os.path.join(directory, "input_RPN.txt")
    
    with open(file_path, 'r') as file:
        for lines in file:
            stack = []
            # Ignore blank line in the input file
            if not lines.strip():
                continue
            # If not blank line
            else:
                for expression in lines.strip():
                    operand = expression.split()
                    for num in operand:
                        # If a number is read, push it to the stack
                        if is_num(num):
                            stack.append(int(num))
                        # If an operator is read, pop the stack and
                        # implement the corresponding calculation
                        elif num in ['+', '-', '*', '/', '%', '^']:
                            first = stack.pop()
                            second = stack.pop()
                            if num == '+': result = second + first
                            if num == '-': result = second - first
                            if num == '*': result = second * first
                            if num == '/': result = second / first
                            if num == '%': result = second % first
                            if num == '^': result = second**first
                            stack.append(result)
                        elif num in ['~']:
                            result = -stack.pop()
                            stack.append(result)
                        # If an invalid character is read, ignore
                        else:
                            continue
                # Print the answer for each expression
                print(stack.pop())
    
main()
