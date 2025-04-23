## Functionality
1. Ability to create and edit n-dimensional figures (where 3 < n < 11);
2. Ability to view and rotate three-dimensional projections of the resulting figures;
3. Ability to generate random multidimensional figures;
4. Ability to save the resulting figures to the file system.
---
Below is development plan for the "Multidimensional Geometrical Editor" project, with all tasks focused on writing code and implementing functionality. The plan is divided into 7 sprints, each containing 3 tasks along with an estimated difficulty level.

### Sprint 1. Basic Project Setup and Application Skeleton
* Task 1.1: Write the initial main file  
Create the main file which establishes the entry point of the application and demonstrates calls to the core modules.  
Difficulty: Low

* Task 1.2: Implement the configuration module  
Write a module that loads the application's settings from a configuration file.  
Difficulty: Low

* Task 1.3: Create the logger  
Develop a logging module that records events to the console or a file, using standard language libraries.  
Difficulty: Medium

### Sprint 2. Basic Classes for n-Dimensional Figures
* Task 2.1: Implement the base class NDShape  
Write the NDShape class representing an n-dimensional figure. The class should include properties for storing vertex coordinates and figure parameters, as well as methods for initialization.  
Difficulty: Medium

* Task 2.2: Methods for handling vertices and edges  
Implement functions within the NDShape class for adding, deleting, and modifying vertex coordinates, and the same for edges.  
Difficulty: Medium

* Task 2.3: Write unit tests for NDShape  
Create a set of unit tests to verify the correctness of the NDShape class methods.  
Difficulty: Low

### Sprint 3. Projecting n-Dimensional Figures into 3D Space
* Task 3.1: Develop the 3D projection function  
Write a function to compute the 3D projection of an n-dimensional figure.  
Difficulty: Hard

* Task 3.2: Implement the rotation algorithm  
Code the functionality to rotate the figure in ,ultidimensional space to change the orientation of the figure in space.  
Difficulty: Hard

* Task 3.3: Implement the Scene module  
Develop a module for composing several objects with specified rotation and perspective on one scene.  
Difficulty: Medium

### Sprint 4. Figure displaying functionality
* Task 4.1: Create shaders  
Write vector and fragment shaders and the same for shadow map.  
Difficulty: Hard

* Task 4.2: Camera movement module  
Implement functions module for camera (First person view) control via keyboard and mouse.  
Difficulty: Hard

* Task 4.3: Integrate 3D projection visualization  
Develop a module to render the computed 3D projection within the application window.  
Difficulty: HELLISHLY HARD

### Sprint 5. Making user ui
* Task 5.1: Create tab system  
Add tabs widget to main window, and create custom tabs module for adding custom tab to it.  
Difficulty: Medium

* Task 5.2: Implement cut / copy / paste / delete / undo / redo system  
Add possibility to revert, repeat changes; add basic editing options.  
Difficulty: Hard

* Task 5.3: Add scene object editor. Part 1  
Add editor widget for changing selected object on scene. There should be features like change name, color, rotation, projection, scale, offset.  
Difficulty: Hard

### Sprint 6. Saving and Loading Figures to/from the File System
* Task 6.1: Serialize the figure into JSON/XML  
Write functions that convert the figure object (NDShape) into a JSON or XML string for saving purposes.  
Difficulty: Easy

* Task 6.2: Implement file save and load functions  
Develop code for writing the serialized string to a file and reading it back with proper deserialization into a figure object.  
Difficulty: Easy

* Task 6.3: Add scene object editor. Part 2   
Add to editor widget shape edititng (dimension, vertices, edges).  
Difficulty: Hard

### Sprint 7. Module Integration, Testing, and Optimization
* Task 7.1: Integrate all modules into a cohesive application  
Write code that integrates all previously developed modules into a unified application with a clear interface.  
Difficulty: Hard

* Task 7.2: Write integration tests  
Implement a suite of tests simulating the complete workflow: creating a figure, editing it, projecting, saving, and loading.  
Difficulty: Hard

* Task 7.3: Refactor, optimize, and document the code  
Perform code refactoring to enhance performance and readability, add detailed comments, and create documentation for all modules.  
Difficulty: Medium
