  - [ ] Implement a new "micro" simulation noise scheme (which could totally change results)
      - [X] for the application
      - [X] for the platform
      - [ ] for both? 

    - [X] Implement energy-specific resource selection schemes
            [X] min watts
            [X] min watts / flops

    - [ ] Modify algorithm scheme:
        - Instead of a single algorithm number (which is the index in the cross-product of all scheme combinations), specify all schemes and always use all algoritnm in their cross-product:
             - task_selection_scheme
             - service_selection
             - core_selection
             - NEW ONE: some resource exclusion scheme for energy stuff!!
                - Perhaps just have a maximum number of hosts to use, which is a configuration parameter of the scheduling scheme? Or perhaps just some fraction of the available compute nodes? Or a fraction of the total node * cores * GHz
                - Or some random subsetting
                - Should be parameterized with some number/fraction... should likely be part of the scheme name? 
                

    - [ ] Come up with an idea for experiments/paper
      - [ ] Compare --no-contention to explore the benefit of using "accurate simulation" instead of "inaccurate model" (should be a small dunk)
      - [ ] Come up with something for energy, probably pretty clear
