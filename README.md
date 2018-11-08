# monsta
This is a simple game for the Parallax Propeller-based WordFire/Quby game console.

This is a maze game with a randomly generated maze. The interior walls of the maze are initially hidden but you discover them by moving around with the arrow keys. When you try to move in the direction where there is a wall, you bump into the wall and the wall becomes visible. There is a goal location that you are trying to reach and there is a monster who is trying to eat you. You have to reach the goal before the monster eats you to win the game. You can pick up various objects along the way. A club will allow you to beat the monster away once and a bomb will let you set a trap for the monster and will also blast away some of the maze walls maybe making it easier for you to reach the goal.

The maze is represented with the following ASCII characters:

- X is a wall
- P is the player
- M is the monster
- ! is a club that you can pick up
- b is an inactive bomb that you can pick up
- B is an active bomb that you have dropped
- ? is a randomizer
- . is a player footprint

The following keys control the game:

- UP/DOWN/LEFT/RIGHT move the player in the corresponding direction
- D drops a bomb and makes it active
- c cheats by unhiding the randomizers
- C cheats by unhiding the randomizers and the maze walls
- B enters beginner mode the next time a game is started
- S starts a game when one is not already in progress
- Q quits the game currently in progress
- - decrements the level
- + increments the level
