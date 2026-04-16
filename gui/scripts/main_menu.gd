
extends Control

var ai_depth: int = 3

func _ready():

	var bg = ColorRect.new()
	bg.color = Color(0.082, 0.082, 0.145)  
	bg.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	add_child(bg)
	

	var center = CenterContainer.new()
	center.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	add_child(center)
	

	var card = PanelContainer.new()
	card.custom_minimum_size = Vector2(460, 600)
	var card_style = StyleBoxFlat.new()
	card_style.bg_color = Color(0.086, 0.129, 0.239, 0.95)  
	card_style.corner_radius_top_left = 16
	card_style.corner_radius_top_right = 16
	card_style.corner_radius_bottom_left = 16
	card_style.corner_radius_bottom_right = 16
	card_style.border_width_left = 2
	card_style.border_width_right = 2
	card_style.border_width_top = 2
	card_style.border_width_bottom = 2
	card_style.border_color = Color(0.3, 0.35, 0.5, 0.4)
	card_style.shadow_color = Color(0, 0, 0, 0.3)
	card_style.shadow_size = 8
	card.add_theme_stylebox_override("panel", card_style)
	center.add_child(card)
	
	var card_margin = MarginContainer.new()
	card_margin.add_theme_constant_override("margin_left", 35)
	card_margin.add_theme_constant_override("margin_right", 35)
	card_margin.add_theme_constant_override("margin_top", 30)
	card_margin.add_theme_constant_override("margin_bottom", 30)
	card.add_child(card_margin)
	
	var vbox = VBoxContainer.new()
	vbox.add_theme_constant_override("separation", 14)
	card_margin.add_child(vbox)
	

	var title = Label.new()
	title.text = "♔  CHESS AI ENGINE"
	title.add_theme_font_size_override("font_size", 28)
	title.add_theme_color_override("font_color", Color(0.95, 0.9, 0.75))
	title.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	vbox.add_child(title)
	
	var subtitle = Label.new()
	subtitle.text = "Minimax + Alpha-Beta Pruning\nDSA Project: Game Tree Search"
	subtitle.add_theme_font_size_override("font_size", 13)
	subtitle.add_theme_color_override("font_color", Color(0.55, 0.6, 0.7))
	subtitle.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	vbox.add_child(subtitle)
	

	vbox.add_child(HSeparator.new())
	

	var play_white_btn = _create_button("♙  Play as White vs Minimax")
	play_white_btn.pressed.connect(_on_play_white_minimax)
	vbox.add_child(play_white_btn)
	
	var play_black_btn = _create_button("♟  Play as Black vs Minimax")
	play_black_btn.pressed.connect(_on_play_black_minimax)
	vbox.add_child(play_black_btn)

	var play_white_maia_btn = _create_button("♙  Play as White vs Maia (1200 ELO)")
	play_white_maia_btn.pressed.connect(_on_play_white_maia)
	vbox.add_child(play_white_maia_btn)
	
	var play_black_maia_btn = _create_button("♟  Play as Black vs Maia (1200 ELO)")
	play_black_maia_btn.pressed.connect(_on_play_black_maia)
	vbox.add_child(play_black_maia_btn)
	
	var play_1v1_btn = _create_button("⚔  Local 1v1")
	play_1v1_btn.pressed.connect(_on_play_1v1)
	vbox.add_child(play_1v1_btn)
	

	vbox.add_child(HSeparator.new())
	

	var diff_header = Label.new()
	diff_header.text = "MINIMAX DIFFICULTY"
	diff_header.add_theme_font_size_override("font_size", 12)
	diff_header.add_theme_color_override("font_color", Color(0.5, 0.55, 0.65))
	diff_header.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	vbox.add_child(diff_header)
	
	var slider = HSlider.new()
	slider.min_value = 1
	slider.max_value = 6
	slider.step = 1
	slider.value = 3
	slider.custom_minimum_size = Vector2(0, 20)
	slider.value_changed.connect(_on_depth_changed)
	vbox.add_child(slider)
	
	var depth_label = Label.new()
	depth_label.name = "DepthLabel"
	depth_label.text = "Depth 3 — Medium (fast, decent)"
	depth_label.add_theme_font_size_override("font_size", 13)
	depth_label.add_theme_color_override("font_color", Color(0.65, 0.7, 0.8))
	depth_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	vbox.add_child(depth_label)
	

	var spacer = Control.new()
	spacer.size_flags_vertical = Control.SIZE_EXPAND_FILL
	vbox.add_child(spacer)
	

	var quit_btn = _create_button("Quit")
	quit_btn.pressed.connect(_on_quit)
	var quit_style = StyleBoxFlat.new()
	quit_style.bg_color = Color(0.35, 0.12, 0.15)
	quit_style.corner_radius_top_left = 6
	quit_style.corner_radius_top_right = 6
	quit_style.corner_radius_bottom_left = 6
	quit_style.corner_radius_bottom_right = 6
	quit_btn.add_theme_stylebox_override("normal", quit_style)
	vbox.add_child(quit_btn)


func _on_play_white_minimax():
	_start_game("w", "ai", "minimax")

func _on_play_black_minimax():
	_start_game("b", "ai", "minimax")

func _on_play_white_maia():
	_start_game("w", "ai", "maia")

func _on_play_black_maia():
	_start_game("b", "ai", "maia")

func _on_play_1v1():
	_start_game("w", "1v1", "minimax")

func _on_quit():
	get_tree().quit()

func _on_depth_changed(value: float):
	ai_depth = int(value)
	var descriptions = {
		1: "Easy (instant)",
		2: "Easy (fast)",
		3: "Medium (fast, decent)",
		4: "Hard (a few seconds)",
		5: "Expert (10-30s)",
		6: "Master (30-60s)"
	}
	var depth_label = find_child("DepthLabel", true, false)
	if depth_label:
		depth_label.text = "Depth " + str(ai_depth) + " — " + descriptions.get(ai_depth, "")

func _start_game(color: String, mode: String, ai_type: String):

	var game_scene = load("res://scenes/game.tscn")
	var game = game_scene.instantiate()
	game.human_color = color
	game.ai_depth = ai_depth
	game.game_mode = mode
	game.opponent_type = ai_type
	get_tree().root.add_child(game)
	get_tree().current_scene = game
	queue_free()


func _create_button(text: String) -> Button:
	var btn = Button.new()
	btn.text = text
	btn.custom_minimum_size = Vector2(0, 44)
	
	var normal_style = StyleBoxFlat.new()
	normal_style.bg_color = Color(0.059, 0.208, 0.376) 
	normal_style.corner_radius_top_left = 8
	normal_style.corner_radius_top_right = 8
	normal_style.corner_radius_bottom_left = 8
	normal_style.corner_radius_bottom_right = 8
	btn.add_theme_stylebox_override("normal", normal_style)
	
	var hover_style = StyleBoxFlat.new()
	hover_style.bg_color = Color(0.325, 0.204, 0.514)  
	hover_style.corner_radius_top_left = 8
	hover_style.corner_radius_top_right = 8
	hover_style.corner_radius_bottom_left = 8
	hover_style.corner_radius_bottom_right = 8
	btn.add_theme_stylebox_override("hover", hover_style)
	
	var pressed_style = StyleBoxFlat.new()
	pressed_style.bg_color = Color(0.914, 0.271, 0.376) 
	pressed_style.corner_radius_top_left = 8
	pressed_style.corner_radius_top_right = 8
	pressed_style.corner_radius_bottom_left = 8
	pressed_style.corner_radius_bottom_right = 8
	btn.add_theme_stylebox_override("pressed", pressed_style)
	
	btn.add_theme_color_override("font_color", Color(0.93, 0.93, 0.97))
	btn.add_theme_font_size_override("font_size", 16)
	
	return btn
