-- sample scene

gold = gr.material({0.9, 0.8, 0.4}, {0.8, 0.8, 0.4}, 25)
grass = gr.material({0.1, 0.7, 0.1}, {0.0, 0.0, 0.0}, 0)
blue = gr.material({0.7, 0.6, 1}, {0.5, 0.4, 0.8}, 25)
stone = gr.material({0.8, 0.7, 0.7}, {0.0, 0.0, 0.0}, 0)
hide = gr.material({0.84, 0.6, 0.53}, {0.3, 0.3, 0.3}, 20)

scene = gr.node('scene')
scene:rotate('X', 30)
scene:translate(6, -2, -15)

-- the con
con = gr.node('con')
scene:add_child(con)
con:translate(-10,-10,-5)
con:rotate('X', 30)

p1 = gr.sphere('p1')
con:add_child(p1)
p1:set_material(gold)
p1:scale(2.0, 4, 2.0)
p1:translate(0, 10.5, 0)

p2 = gr.sphere('p2')
con:add_child(p2)
p2:set_material(gold)
p2:scale(2.0, 4, 2.0)
p2:translate(4.0, 10, 0)

c1 = gr.cube('c1')
con:add_child(c1)
c1:set_material(hide)
c1:scale(5.8, 2.9, 2.8)
c1:translate(3.0, 6, 9)
c1:rotate('X', -30)
c1:rotate('Y', 5)

-- mickey
mickey = gr.sphere( 'mickey')
con:add_child(mickey)
mickey:scale(0.5, 0.5, 0.5)
mickey:translate(6.0, 11, 1)
mickey:set_material(stone)

-- the floor
plane = gr.mesh( 'plane', 'plane.obj' )
scene:add_child(plane)
plane:set_material(blue)
plane:scale(30, 30, 30)

-- icosa
poly = gr.mesh( 'poly', 'icosa.obj' )
scene:add_child(poly)
poly:translate(-3, 5, 0)
poly:set_material(grass)



-- The lights
l1 = gr.light({200,200,400}, {0.8, 0.8, 0.8}, {1, 0, 0})
l2 = gr.light({0, 100, -20}, {0.4, 0.4, 0.8}, {1, 0, 0})

gr.render(scene, 'sample.png', 512, 512,
	  {0, 0, 0,}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.4, 0.4, 0.4}, {l1, l2})
