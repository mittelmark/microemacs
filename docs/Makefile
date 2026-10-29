

default:
	tmdoc emf-tutorial.md .emf-tutorial-woven.md --mode weave --toc false
	mndoc .emf-tutorial-woven.md emf-tutorial.html --css mini.css --base64 false
	awk '{ sub(/[[:space:]]+$$/, ""); if (NF) { for (i = 0; i < blank; i++) print ""; print; blank = 0 } else blank++ }' emf-tutorial.html > .emf-tutorial-clean.html
	mv .emf-tutorial-clean.html emf-tutorial.html
	rm -f .emf-tutorial-woven.md
