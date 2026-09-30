Game!!

Monday, 5/16, 5:00 pm - 1:00 am

WHO: Ricardo, Michelle, Pat, and Kevin
WHAT: Ricardo, Pat, and Kevin worked on getting the keyboard controls to work. They were especially focused on being able to handle two buttons pressed at once. Michelle created enemy structs and worked on getting them to appear on screen at random points after set time intervals.
BUGS: Needed to figure out how to press multiple buttons at once.
RESOURCES USED: NA

Tuesday, 5/17, 

WHO: Ricardo and Kevin
WHAT: Implemented a following force for the missiles. This allowed everything to pursue the spaceship. Also made movement of spaceships smoother.
BUGS: NA
RESOURCES USED: NA

Thursday, 5/19, 7:00 - 9:00pm

WHO: Michelle, Ricardo, Kevin, and Pat
WHAT: Michelle worked on plotting the different types of enemies, there was a bug with switch cases that caused things to draw over each other. Kevin worked on hit points and health within enemy struct. Pat worked on designing the new spaceship guy and adding him in. Ricardo began levels.c and starting thinking through different enemy combos for levels.
BUGS: liveshare is buggy as hell. switch cases needs a break statement between cases. 
RESOURCES USED: NA


Thursday, 5/19, 9:00 - 11:30pm

WHO: Michelle and Pat
WHAT: Michelle worked on getting enemies to shoot bullets every time interval set at initialization of the enemy. They can now follow the spaceship and shoot at specified time intervals. Pat worked on creating the design for the turret. Pat's computer was unable to pull or make though, so they spent time working through that with Neil.
BUGS: dt is a little weird to use, apparently it can be negative? But also Pat's entire VS Code was down the whole time so they worked on Notepad literally.
RESOURCES USED: TA Neil


Thursday, 5/23, 7:45 - 10:30pm

WHO: Michelle, Kevin and Pat
WHAT: Pat finished turret rotation. Kevin worked on decreasing health and body removal when enemy killed. Michelle and Kevin worked on trying to get the moving enemy to follow a given circular path. We finished a good portion of the game, it now looks pretty cool!! :D
BUGS: Much harder to make enemy move in a circular path from it's given position than we expected.
RESOURCES USED: NA


Wednesday, 5/25, 8:30 - 11:30pm

WHO: Ricardo, Michelle, Kevin and Pat
WHAT: Ricardo worked on project by himself for the first hour. Michelle joined around the one hour mark, and then Kdo and Pat also pulled up later. Ricardo worked on normalizing the shapes, fixed the not shooting issue, and fixed his OD bug with missiles and bodies collision. Michelle worked on timing out the missiles after an interval and exploding an enemy and replacing it with a circle of bullets. Pat and Kevin worked on getting a health bar to appear on screen and trying to update it.
BUGS: New enemies must be linked to existing missiles, otherwise missiles will only collide with the existing enemies at their point of initialization. Another bug is trying to shoot a ring of bullets from an exploding enemy, enemy_shoot is not working with explode function for some reason.
RESOURCES USED: NA


Thursday, 5/26, 9:00 - 11:30pm

WHO: Pat, Michelle, Kevin, and Ricardo
WHAT: Pat and Michelle pulled up to OH at 9. Michelle worked on exploding enemies into a circle of bullets, future steps will be to change the enemy struct so that they fire at different velocities for different enemies. Pat finished health bar. Ricardo and Kevin pulled up later. Ricardo and Michelle worked on fixing bugs with exception thrown when the spaceship gets killed. Pat and Kevin worked on pause button and will work integrating old key handler next.
RESOURCES USED: TA Neil

Tuesday, 5/31, 6:00 pm - 12:30 am

WHO: Pat, Michelle, Kevin, and Ricardo
WHAT: Keven tried to figure out how to add texts. Michelle tried adding sounds. Pat tried adding a pause screen. None of our efforts worked. We ran into error after error, with SDL references being undefined, then all the asset files couldn't be loaded iproperly/were null
RESOURCES USED: TA Neil

Wednesday, 6/1, 10 pm - 11:30

WHO: Kevin, Michelle, Pat, Ricardo

WHAT: Kevin and Sniggy got something to finally print out on the text!!!! Michelle and Sniggy worked on trying to get collision sounds to work, still showing null chunks though. Pat worked on designing Celestial Runner font. Ricardo planned out levels.
BUGS: We were clearing the screen in sdl render scene and we were clearing the scene in the beginning of main, so nothing could have ever possibly pop up.
RSOURCES USED: Sniggy

Thursday, 6/2/2022, 6:30 - 8:30pm

WHO: Michelle
WHAT: Merged the text and sound edits with the updated screen stuff. Worked on debugging sound with new sound files and other bugs that came with merging.
BUGS: Corrupted sound file got updated, apparently make_text function needed a name change and that allowed the function to work with merge.
RESOURCES USED: TA Neil and Sniggy

Thursday, 6/2/2022, 8:30 - 

WHO: Michelle
WHAT: 
BUGS: 
RESOURCES USED: 

