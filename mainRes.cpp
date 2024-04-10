#include <string>
#include <iostream>

using namespace std;

int _print()
{
	cout;
	return 0;
}
int _print(int input)
{
	cout << input;
	return 0;
}
int _print(string input)
{
	cout << input;
	return 0;
}
int _print(bool value)
{
	if (value)
		cout << "True";
	else
		cout << "False";
	return 0;
}

int _inputNum()
{
	int inValue;
	cin >> inValue;
	return inValue;
}

string _inputString()
{
	string inValue;
	cin >> inValue;
	return inValue;
}

#include <string>
#include <iostream>

using namespace std;

int _add(int a, int b)
{
	return a + b;
}

int _sub(int a, int b)
{
	return a - b;
}

int _mul(int a, int b)
{
	return a * b;
}

int _div(int a, int b)
{
	return a / b;
}

int _pow(int a, int b)
{
	return pow(a, b);
}

bool _equals(int a, int b)
{
	return a == b;
}

bool _or(bool a, bool b)
{
	return a || b;
}

bool _and(bool a, bool b)
{
	return a && b;
}

#include <string>
#include <iostream>

using namespace std;

string _toString(int i)
{
	return to_string(i);
}

string _toString(bool b)
{
	return to_string(b);
}

int _toInt(string str)
{
	return stoi(str);
}

string _concat(string s1, string s2)
{
	return s1 + s2;
}

string _concat(string s1, int s2)
{
	return s1 + _toString(s2);
}
string _concat(int s1, string s2)
{
	return _toString(s1) + s2;
}

bool _equals(string a, string b)
{
	return a == b;
}

int _fibPart(int _prev, int _curr, int _n, int _index)
{
	if (_equals(_n, _index))
{
	return _add(_prev, _curr);
}
	return _fibPart(_curr, _add(_curr, _prev), _n, _add(_index, 1));
}

int _fibonacci(int _n)
{
	int _prev;	_prev = 0;	int _curr;	_curr = 1;	return _fibPart(_prev, _curr, _n, 0);
}

bool _body(int _index)
{
	_print((string)"Write n to calculate fibonacci: ");	string _n;	_n = _inputString();	_print((string)"\n");	if (_or(_equals(_n, (string)"e"), _equals(_n, (string)"exit")))
{
	return true;
}
	_print((string)"Fibonacci for ");	_print(_n);	_print((string)": ");	_print(_fibonacci(_toInt(_n)));	_print((string)"\n");	return false;
}

int _loop(int _index, int _max, bool(*_loopBody)(int))
{
	if (_loopBody(_index))
{
	return 0;
}
	if (!(_equals(_index, _max)))
{
	_loop(_add(_index, 1), _max, _loopBody);
}
	return 0;
}

int main() {
_print((string)"To exit write (exit) or (e)\n");
_loop(0, -1, _body);

return 0; 
 }