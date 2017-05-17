-- puppet.lua
-- A simplified puppet without posable joints, but that
-- looks roughly humanoid.

rootnode = gr.node('root')
--rootnode:rotate('y', 15.0)
rootnode:scale( 0.25, 0.25, 0.25 )
rootnode:translate(0.0, 0.0, -1.0)

red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)
yellow = gr.material({1.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)

torso = gr.mesh('sphere', 'torso')
rootnode:add_child(torso)
torso:set_material(red)
torso:scale(0.5, 0.5, 0.5);



head = gr.mesh('sphere', 'head')
torso:add_child(head)
head:scale(1.0/0.5, 1.0/0.5, 1.0/0.5)
head:scale(0.3, 0.3, 0.3)
head:translate(0.0, 1.8, 0.0)
head:set_material(yellow)

leftEye = gr.mesh('sphere', 'leftEye')
head:add_child(leftEye)
leftEye:scale(0.2, 0.2, 0.2)
leftEye:translate(-0.4, 0.0, 0.9)
leftEye:set_material(red)

rightEye = gr.mesh('sphere', 'rightEye')
head:add_child(rightEye)
rightEye:scale(0.2, 0.2, 0.2)
rightEye:translate(0.4, 0.0, 0.9)
rightEye:set_material(red)

neck = gr.mesh('sphere', 'neck')
torso:add_child(neck)
neck:scale(1.0/0.5, 1.0, 1.0/0.5)
neck:scale(0.10, 0.5, 0.10)
neck:translate(0.0, 1.2, 0.0)
neck:set_material(blue)


ears = gr.mesh('sphere', 'ears')
head:add_child(ears)
ears:scale(1.2, 0.08, 0.08)
ears:set_material(red)
ears:set_material(blue)

leftEye = gr.mesh('cube', 'leftEye')
head:add_child(leftEye)
leftEye:scale(0.2, 0.1, 0.1)
leftEye:translate(-0.2, 0.2, 0.5)
leftEye:set_material(blue)

rightEye = gr.mesh('cube', 'rightEye')
head:add_child(rightEye)
rightEye:scale(0.2, 0.1, 0.1)
rightEye:translate(0.2, 0.2, 0.5)
rightEye:set_material(blue)

leftShoulder = gr.mesh('sphere', 'leftShoulder')
torso:add_child(leftShoulder)
leftShoulder:scale(1/0.5, 1/0.5, 1/0.5)
leftShoulder:scale(0.2, 0.2, 0.2)
leftShoulder:translate(-0.8, 0.7, 0.0)
leftShoulder:set_material(blue)

leftUpperArm = gr.mesh('sphere', 'leftUpperArm')
leftShoulder:add_child(leftUpperArm)
leftUpperArm:scale(1.0, 1.0, 1.0)
leftUpperArm:scale(1.2, 0.4, 0.4)
leftUpperArm:rotate('z', 50);
leftUpperArm:translate(-1.0, -1.0, 0.0)
leftUpperArm:set_material(red)

leftForeArm = gr.mesh('sphere', 'leftForeArm')
leftUpperArm:add_child(leftForeArm)
leftForeArm:scale(1.0, 1.0, 1.0)
leftForeArm:scale(1.0, 0.5, 0.5)
leftForeArm:rotate('z', 0);
leftForeArm:translate(-1.4, 0.0, 0.0)
leftForeArm:set_material(yellow)

leftHand = gr.mesh('sphere', 'leftHand')
leftForeArm:add_child(leftHand)
leftHand:scale(1/6.0, 1.0, 1.0)
leftHand:scale(4.0, 4.0, 4.0)
leftHand:translate(-1.2, 0.0, 0.0)
leftHand:set_material(green)

rightShoulder = gr.mesh('sphere', 'rightShoulder')
torso:add_child(rightShoulder)
rightShoulder:scale(1/0.5, 1/0.5, 1/0.5)
rightShoulder:scale(0.2, 0.2, 0.2)
rightShoulder:translate(0.8, 0.7, 0.0)
rightShoulder:set_material(blue)

rightUpperArm = gr.mesh('sphere', 'rightUpperArm')
rightShoulder:add_child(rightUpperArm)
rightUpperArm:scale(1.0, 1.0, 1.0)
rightUpperArm:scale(1.2, 0.4, 0.4)
rightUpperArm:rotate('z', -50);
rightUpperArm:translate(1.0, -1.0, 0.0)
rightUpperArm:set_material(red)

rightForeArm = gr.mesh('sphere', 'rightForeArm')
rightUpperArm:add_child(rightForeArm)
rightForeArm:scale(1.0, 1.0, 1.0)
rightForeArm:scale(1.0, 0.5, 0.5)
rightForeArm:rotate('z', 0);
rightForeArm:translate(1.4, 0.0, 0.0)
rightForeArm:set_material(yellow)

rightHand = gr.mesh('sphere', 'rightHand')
rightForeArm:add_child(rightHand)
rightHand:scale(1/6.0, 1.0, 1.0)
rightHand:scale(4.0, 0.2, 4.0)
rightHand:translate(1.2, 0.0, 0.0)
rightHand:set_material(green)



leftHip = gr.mesh('sphere', 'leftHip')
torso:add_child(leftHip)
leftHip:scale(1/0.5,1.0,1/0.5);
leftHip:scale(0.21, 0.21, 0.21)
leftHip:translate(-0.38, -1.0, 0.0)
leftHip:set_material(blue)

leftThigh = gr.mesh('sphere', 'lefThigh')
leftHip:add_child(leftThigh)
leftThigh:scale(1.0, 2.0 ,1.0)
leftThigh:translate(0,-2.5,0)
leftThigh:set_material(red)

leftCalf = gr.mesh('sphere', 'lefCalf')
leftThigh:add_child(leftCalf)
leftCalf:scale(0.7,2.0,0.7)
leftCalf:translate(0,-2.5,0)
leftCalf:set_material(green)

leftFoot = gr.mesh('sphere', 'lefFoot')
leftCalf:add_child(leftFoot)
leftFoot:scale(1.0/0.7, 1.0, 1.0/0.7)
leftFoot:scale(0.5,0.3,2.0)
leftFoot:translate(0,-1.2,1.0)
leftFoot:set_material(yellow)

rightHip = gr.mesh('sphere', 'rightHip')
torso:add_child(rightHip)
rightHip:scale(1/0.5,1.0,1/0.5);
rightHip:scale(0.21, 0.21, 0.21)
rightHip:translate(0.38, -1.0, 0.0)
rightHip:set_material(blue)

rightThigh = gr.mesh('sphere', 'righthigh')
rightHip:add_child(rightThigh)
rightThigh:scale(1.0, 2.0 ,1.0)
rightThigh:translate(0,-2.5,0)
rightThigh:set_material(red)

rightCalf = gr.mesh('sphere', 'lefCalf')
rightThigh:add_child(rightCalf)
rightCalf:scale(0.7,2.0,0.7)
rightCalf:translate(0,-2.5,0)
rightCalf:set_material(green)

rightFoot = gr.mesh('sphere', 'lefFoot')
rightCalf:add_child(rightFoot)
rightFoot:scale(1.0/0.7, 1.0, 1.0/0.7)
rightFoot:scale(0.5,0.3,2.0)
rightFoot:translate(0,-1.2,1.0)
rightFoot:set_material(yellow)


return rootnode
