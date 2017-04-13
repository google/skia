recipes_test:
	python recipes.py simulation_test

recipes_train:
	python recipes.py simulation_test train

tasks_test:
	go run gen_tasks.go --test

tasks_train:
	go run gen_tasks.go

python_tests:
	python -m unittest discover -s . -p *_test.py

.PHONY: test
test: recipes_test tasks_test python_tests

.PHONY: train
train: recipes_train tasks_train
