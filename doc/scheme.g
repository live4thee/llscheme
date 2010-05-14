// The ANTLR grammar file for the Scheme language.
grammar scheme;

form	:	CONSTANT | symbol | spform | apply;

CONSTANT:	'#t' | '#f';
LETTER	:	('a'..'z'|'A'..'Z')+;
DIGIT	:	'0'..'9'+;

symbol	:	LETTER ( LETTER | DIGIT | '-' | '!' | '?' )*;

spform	:	if | define | lambda;
if	:	'(' 'if' form form form? ')';
define	:	'(' 'define' symbol form ')';
lambda	:	'(' 'lambda' '(' args ')' form? ')';

args	:	symbol*
	|	symbol+ '.' symbol
		;
apply	:	'(' form  form? ')';

WS	:	(' ' | '\t' | '\r' | '\n') {skip();};
