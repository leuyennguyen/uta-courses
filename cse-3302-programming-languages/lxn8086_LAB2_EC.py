#	Le Uyen Nguyen
#	100 171 8086
#	04/01/2021
#	Windows 10 - Python 3.9.2 - IDLE Shell

import os

# Function is_num() takes input n, return a boolean value,
# which is true if n is an integer; else, false
def is_num(n):
    try:
        int(n)
        return True
    except ValueError:
        return False

# Function isEmpty() checks if stack is empty
def isEmpty(stack):
    return len(stack) == 0

# Function precedenceCheck() checks the precedence between
# the new input and the stack top
def precedenceCheck(stack, i):
    precedence = {'+': 1,
                  '-': 1,
                  '*': 2,
                  '/': 2,
                  }
    if stack[len(stack)-1] == '(':
        return False
    if precedence[i] <= precedence[stack[len(stack)-1]]:
        return True
    else:
        return False

# Function calculation() calculates the result of the given expression
def calculation(expression):
    stack = []
    for num in expression:
        # If a number is read, push it to the stack
        if is_num(num):
            stack.append(int(num))
        # If an operator is read, pop the stack and
        # implement the corresponding calculation
        elif num in ['+', '-', '*', '/']:
            first = stack.pop()
            second = stack.pop()
            if num == '+': result = second + first
            if num == '-': result = second - first
            if num == '*': result = second * first
            if num == '/': result = second / first
            stack.append(result)
        # If an invalid character is read, ignore
        else:
            continue
    return stack.pop()

def main():
    # Get the file path
    directory = os.path.dirname(os.path.abspath(__file__))
    file_path = os.path.join(directory, "input_RPN_EC.txt")
    
    with open(file_path, 'r') as file:
        for lines in file:
            # Ignore blank line in the input file
            if not lines.strip():
                continue
            # If not blank line
            else:
                stack = []
                # output stores the postfix-notation expression
                output = ''
                
                # counter keeps track with the line number in the file
                # if the expresion given at the line is invalid, counter
                # is used to display the line number
                counter = 0
                
                for expression in lines.strip():
                    for num in expression:
                        
                        # The operand is added to the postfix string
                        if is_num(num):
                            output += num
                            
                        # The postfix string contains no parenthesis
                        # If '(' is read, it is added to the stack
                        elif num == '(':
                            stack.append(num)

                        # If ')' is read:
                        elif num == ')':
                            # Pop the stack until the corresponding '(' is removed
                            # Append the operators read from the stack to the postfix string
                            while ((not isEmpty(stack)) and (stack[-1] != '(')):
                                o = stack.pop()
                                output += o

                            # If no corresponding '(' presented, the expression is invalid
                            if isEmpty(stack):
                                output = 'Invalid expression in line '+str(counter+1)+ ': '
                                break
                            else:
                                stack.pop()

                        # If a space is read, ignore
                        elif num == ' ':
                            continue

                        # If the valid operator is read, keep check its precedence compared to
                        # those already on the stack. Whichever operator has higher or equal
                        # precedence will be append to the postfix string
                        else:
                            while (not isEmpty(stack) and precedenceCheck(stack, num)):
                                output += stack.pop()
                            stack.append(num)

                # If the expression is completely read, pop and append all operators left on the stack 
                while not isEmpty(stack):
                    output += stack.pop()

                # Print the postfix-notation expression along with its result
                print('The RPN: '+output)
                print('Result = '+str(calculation(output)))

                # Update line number
                counter+=1

main()
