  - [X] Make it possible to have algorithm selection triggered whenever a workflow level completes
  - [ ] Implement some bi-objective energy/makespan thing
    - [ ] Implement a new "micro" simulation noise scheme (which could totally change results)
        - [X] for the application
        - [X] for the platform
        - [ ] for both? 
    - [X] Have children report on energy as well as makespan
    - [X] Implement a simple scheme
    - [X] Add watts to cluster descriptions
    - [X] Implement energy-specific resource selection schemes
            [X] min watts
            [X] min watts / flops
    - [ ] Come up with an idea for experiments/paper
      - [ ] We need to have some resource exclusion scheme? 
            Perhaps just have a maximum number of hosts to use, which is a configuration parameter of the scheduling scheme? Or perhaps just some fraction of the available compute nodes? Or a fraction of the total node * cores * GHz
    
