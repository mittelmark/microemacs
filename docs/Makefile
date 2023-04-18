

default:
	pandoc --filter pantcl emf-tutorial.md -o temp.html -s --css mini.css \
		--syntax-definition emf.xml
	htmlark temp.html -o emf-tutorial.html
	rm temp.html

