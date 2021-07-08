# TODO: update to the CURRENT assignment and questions
# WARNING: each question *must* be in a sub-directory with the given name
ASSIGNMENT = c1
QUESTIONS = q1

# TODO: make sure you have a readme file!
README = readme.md

#EXTS = c h py md txt
EXTS = c h md txt
#EXTS = c py md

# DO NOT modify anything beyond this point (unless you really know what you're doing)

COURSE = 4140

USER = $(shell whoami)

.PHONY: questions $(QUESTIONS)

questions: $(QUESTIONS)

# build each of the questions -- each question *must* have its own Makefile
# see course website for the Makefile to use with questions
$(QUESTIONS):
	@$(MAKE) -C $@

.PHONY: handin
handin: questions $(README)
	@echo
	@rm -rf $(ASSIGNMENT)
	@mkdir $(ASSIGNMENT)
	@for q in $(QUESTIONS); \
	do \
		echo "Preparing" $$q "for submission..."; \
		mkdir $(ASSIGNMENT)/$$q; \
		echo "Made directory for" $$q "for submission..."; \
		cp -n $$q/Makefile $(ASSIGNMENT)/$$q/; \
		for ext in $(EXTS); \
		do \
			cp -n $$q/*.$$ext $(ASSIGNMENT)/$$q/; \
		done \
	done
	@echo "done processing questions"
	@cp -n $(README) $(ASSIGNMENT)
	@echo "Handing in" $(ASSIGNMENT)"..."
	handin $(COURSE) $(ASSIGNMENT) $(ASSIGNMENT) | tee $(ASSIGNMENT)/receipt.txt
	@echo
	@echo "Submission complete. Confirmation details saved to" $(ASSIGNMENT)"/receipt.txt."

.PHONY: clean
clean: 
	@for q in $(QUESTIONS); \
	do \
		make -C $$q clean; \
	done
	rm -rf $(ASSIGNMENT)
