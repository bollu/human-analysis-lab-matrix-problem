.PHONY: main.pdf
main.pdf: *.tex
	latexmk  -shell-escape -pdf
	latexmk -c

watch: *.tex
	latexmk -pdf -pvc main.tex && latexmk -c
