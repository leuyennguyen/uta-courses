/*
	Le Uyen Nguyen
	100 171 8086
	03/03/2021
*/

/*
1.	Start with an array called inputtable. The array should have numbers between 1 and 10.
*/

var inputtable = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
console.log(inputtable);

/*----------------------------------------------------------------------------------------
2.	Use inputtable from step 1 to create the following:
	a.	Set of multiples of 5 between 1 and 51. Name it fiveTable
	b.	Set of multiples of 13 between 1 and 131. Name it thirteenTable
	c.	Set of squares of the numbers in inputtable. Name it squaresTable
*/

// Multiples of 5 in (1, 51) is the products of 5 and each element in inputtable
var fiveTable = inputtable.map(v => v*5);
console.log(fiveTable);

// Multiples of 13 in (1, 131) is the products of 13 and each element in inputtable
var thirteenTable = inputtable.map(v => v*13);
console.log(thirteenTable);

// Multiply each number in inputtable by itself to get its square
var squaresTable = inputtable.map(v => v*v);
console.log(squaresTable);

/*----------------------------------------------------------------------------------------
3.	Get the odd multiples of 5 between 1 and 100. 5, 15, … 
*/

// Product of 5 and each element in inputtable is a multiple of 5 in range (1, 51). If
// each product is multiplied by 2, we will get the even multiple of 5 in range (1, 100).
// To get the odd multiples of 5, we subtract 5 from each even multiple.
var oddMultiplesOfFive = inputtable.map(v => (v*5)*2-5);
console.log(oddMultiplesOfFive);

/*----------------------------------------------------------------------------------------
4.	Get the sum of even multiples of 7 between 1 and 100. 
a.	Example, find the multiples and then sum them: 14 + 28+… 
*/

// Doubling the product of 7 and each element in inputtable is the even multiple of 7
// in range (1, 140).
var evenMultiplesOfSeven = inputtable.map(v => (v*7)*2);

// isValid() returns true if the number passed in less than or equal 100; else, false
var isValid = x => x <= 100;

// validValue is an array containing all even multiples of 7 in range (1, 100)
var validValue = evenMultiplesOfSeven.filter(isValid);
var add = (x, y) => x + y;
var sum = validValue.reduce(add, 0);
console.log(sum);

/*----------------------------------------------------------------------------------------
5.	Use currying to rewrite the function below: -
function cylinder_volume(r, h){
	var volume = 0.0;
	volume = 3.14 * r * r * h;
	return volume;
}
Use r = 5 and h = 10 to call your curried function.
*/

// cylinder_volume() takes one parameter at a time, r, then h, and returns volume
// calculated by pi*r^2*h
function cylinder_volume(r) {
	return function(h) {
		return Math.PI * r * r * h;
	}
}

var volume = cylinder_volume(5)(10);
console.log(volume);

/*----------------------------------------------------------------------------------------
6.	Use the following code to take advantage of closures to wrap content with HTML tags, 
specifically show an HTML table consisting of a table row that has at least one table 
cell/element. You can use the console to output your results.
*/

makeTag = function(beginTag, endTag){
	return function(textcontent){
		return beginTag + textcontent +endTag;
	}
}

var a = makeTag("<table>\n\t<tr>\n\t\t<th>", "</th>\n")("Assignment");
var b = makeTag("\t\t<th>", "</th>\n\t</tr>\n\t<tr>\n")("Due date");
var c = makeTag("\t\t<td>", "</td>\n")("Lab 1");
var d = makeTag("\t\t<td>", "</td>\n\t</tr>\n\t<tr>\n")("March 03");
var e = makeTag("\t\t<td>", "</td>\n")("Homework 3");
var f = makeTag("\t\t<td>", "</td>\n\t</tr>\n</table>")("March 08");
console.log(a, b, c, d, e, f);

/*----------------------------------------------------------------------------------------
8.	(Extra credit) Do the ‘generic’ version of questions 3 and 4, meaning the target 
multiple must not be hard coded – first odd or even and then the number whose multiples 
(in range 1 to 100) you want.
*/

// Function generic() take the upper and lower bound, and create an array of range.
// It returns the function multiple(), which takes the number whose needs to 
// calculate multiples, and the mode (even/odd). Function multiple() then returns
// the array of valid multiples of the base number.
function generic(lowerBound, upperBound) {
	var valueTable = Array.from(Array(upperBound - lowerBound)).map((e,i)=>i+1);
	return function multiple(base, evenOrOdd) {
		var isEven = x => x % 2 === 0;
		var isOdd = x => x % 2 !== 0;
		var isValid = x => x <= upperBound;
		if (evenOrOdd === "even") {
			return valueTable.map(x => x*base).filter(isValid).filter(isEven);
		}
		else if (evenOrOdd === "odd") {
			return valueTable.map(x => x*base).filter(isValid).filter(isOdd);
		}
		else {
			return "Invalid";
		}
	}
}
console.log(generic(1, 100)(5, "odd"));
