INPUTNAME= ropterminator
PAPERNAME= ropterminator

BIBTEX = bibtex
PDFLATEX = pdflatex

DEPENDENCIES = $(shell ls *.tex *.bib) 

# Default to compile when no specification is provided:
all: $(PAPERNAME).pdf

$(PAPERNAME).pdf: $(DEPENDENCIES)
	$(PDFLATEX) -synctex=1 $(INPUTNAME).tex
	$(BIBTEX) $(INPUTNAME).aux
	#./fixbibtex.perl $(INPUTNAME).bbl
	$(PDFLATEX) -synctex=1 -interaction=nonstopmode $(INPUTNAME).tex
	$(PDFLATEX) -synctex=1 -interaction=nonstopmode $(INPUTNAME).tex

clean:
	rm -f $(PAPERNAME).pdf *.tgz *.dvi *.aux *.log *.blg *.bbl *~
