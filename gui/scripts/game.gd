
extends Control


var human_color: String = "w"  
var ai_depth: int = 3
var game_mode: String = "ai"    
var opponent_type: String = "minimax" 


var engine: Node = null
var board: Node2D = null
var move_history: Array = []
var all_legal_moves: PackedStringArray = PackedStringArray()
var current_side: String = "w"
var is_game_over: bool = false
var is_ai_thinking: bool = false


var status_label: Label = null
var history_label: RichTextLabel = null
var eval_label: Label = null
var captured_white_label: Label = null
var captured_black_label: Label = null
var thinking_anim: Label = null
var hint_label: RichTextLabel = null
var back_button: Button = null


func _ready():

	var bg = ColorRect.new()
	bg.color = Color(0.082, 0.082, 0.145)  
	bg.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	add_child(bg)
	

	var margin = MarginContainer.new()
	margin.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	margin.add_theme_constant_override("margin_left", 20)
	margin.add_theme_constant_override("margin_right", 20)
	margin.add_theme_constant_override("margin_top", 15)
	margin.add_theme_constant_override("margin_bottom", 15)
	add_child(margin)
	
	var hbox = HBoxContainer.new()
	hbox.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	hbox.add_theme_constant_override("separation", 20)
	margin.add_child(hbox)
	

	var left_panel = _create_panel()
	left_panel.custom_minimum_size = Vector2(220, 0)
	left_panel.size_flags_vertical = Control.SIZE_EXPAND_FILL
	hbox.add_child(left_panel)
	
	var left_vbox = VBoxContainer.new()
	left_vbox.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	left_vbox.add_theme_constant_override("separation", 12)
	var left_margin = MarginContainer.new()
	left_margin.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	left_margin.add_theme_constant_override("margin_left", 14)
	left_margin.add_theme_constant_override("margin_right", 14)
	left_margin.add_theme_constant_override("margin_top", 14)
	left_margin.add_theme_constant_override("margin_bottom", 14)
	left_panel.add_child(left_margin)
	left_margin.add_child(left_vbox)
	

	var title = Label.new()
	title.text = "♔ CHESS AI"
	title.add_theme_font_size_override("font_size", 22)
	title.add_theme_color_override("font_color", Color(0.9, 0.85, 0.7))
	title.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	left_vbox.add_child(title)
	

	var mode_label = Label.new()
	if game_mode == "ai":
		var opp = "vs Minimax (Depth " + str(ai_depth) + ")"
		if opponent_type == "maia":
			opp = "vs Maia (1200 ELO)"
		mode_label.text = opp + "\nYou: " + ("White" if human_color == "w" else "Black")
	else:
		mode_label.text = "Local 1v1"
	mode_label.add_theme_font_size_override("font_size", 13)
	mode_label.add_theme_color_override("font_color", Color(0.6, 0.65, 0.75))
	mode_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	left_vbox.add_child(mode_label)
	

	left_vbox.add_child(HSeparator.new())
	

	var cap_title = Label.new()
	cap_title.text = "CAPTURED"
	cap_title.add_theme_font_size_override("font_size", 12)
	cap_title.add_theme_color_override("font_color", Color(0.5, 0.55, 0.65))
	left_vbox.add_child(cap_title)
	
	captured_white_label = Label.new()
	captured_white_label.text = "White lost: —"
	captured_white_label.add_theme_font_size_override("font_size", 13)
	captured_white_label.add_theme_color_override("font_color", Color(0.85, 0.85, 0.85))
	captured_white_label.autowrap_mode = TextServer.AUTOWRAP_WORD
	left_vbox.add_child(captured_white_label)
	
	captured_black_label = Label.new()
	captured_black_label.text = "Black lost: —"
	captured_black_label.add_theme_font_size_override("font_size", 13)
	captured_black_label.add_theme_color_override("font_color", Color(0.85, 0.85, 0.85))
	captured_black_label.autowrap_mode = TextServer.AUTOWRAP_WORD
	left_vbox.add_child(captured_black_label)
	

	left_vbox.add_child(HSeparator.new())
	

	eval_label = Label.new()
	eval_label.text = "Eval: 0.00"
	eval_label.add_theme_font_size_override("font_size", 13)
	eval_label.add_theme_color_override("font_color", Color(0.7, 0.75, 0.85))
	left_vbox.add_child(eval_label)

	var hint_title = Label.new()
	hint_title.text = "HINTS"
	hint_title.add_theme_font_size_override("font_size", 12)
	hint_title.add_theme_color_override("font_color", Color(0.5, 0.55, 0.65))
	left_vbox.add_child(hint_title)
	
	hint_label = RichTextLabel.new()
	hint_label.bbcode_enabled = true
	hint_label.fit_content = true
	hint_label.add_theme_font_size_override("normal_font_size", 13)
	hint_label.add_theme_color_override("default_color", Color(0.75, 0.8, 0.65))
	left_vbox.add_child(hint_label)
	

	var spacer = Control.new()
	spacer.size_flags_vertical = Control.SIZE_EXPAND_FILL
	left_vbox.add_child(spacer)
	

	back_button = _create_button("← Main Menu")
	back_button.pressed.connect(_on_back_pressed)
	left_vbox.add_child(back_button)
	

	var board_container = CenterContainer.new()
	board_container.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	board_container.size_flags_vertical = Control.SIZE_EXPAND_FILL
	hbox.add_child(board_container)
	
	var board_wrapper = Control.new()
	board_wrapper.custom_minimum_size = Vector2(board.BOARD_PX if board else 600, board.BOARD_PX if board else 600)
	board_container.add_child(board_wrapper)
	

	var BoardScene = preload("res://scripts/board.gd")
	board = Node2D.new()
	board.set_script(BoardScene)
	board.is_flipped = (human_color == "b" and game_mode == "ai")
	board_wrapper.add_child(board)
	board.move_attempt.connect(_on_move_attempt)
	

	var status_container = VBoxContainer.new()
	status_container.position = Vector2(0, 610)
	status_container.custom_minimum_size = Vector2(600, 40)
	board_wrapper.add_child(status_container)
	
	status_label = Label.new()
	status_label.text = "Starting game..."
	status_label.add_theme_font_size_override("font_size", 16)
	status_label.add_theme_color_override("font_color", Color(0.9, 0.9, 0.75))
	status_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	status_container.add_child(status_label)
	
	thinking_anim = Label.new()
	thinking_anim.text = ""
	thinking_anim.add_theme_font_size_override("font_size", 13)
	thinking_anim.add_theme_color_override("font_color", Color(0.6, 0.7, 0.9))
	thinking_anim.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	status_container.add_child(thinking_anim)
	

	var right_panel = _create_panel()
	right_panel.custom_minimum_size = Vector2(200, 0)
	right_panel.size_flags_vertical = Control.SIZE_EXPAND_FILL
	hbox.add_child(right_panel)
	
	var right_margin = MarginContainer.new()
	right_margin.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	right_margin.add_theme_constant_override("margin_left", 14)
	right_margin.add_theme_constant_override("margin_right", 14)
	right_margin.add_theme_constant_override("margin_top", 14)
	right_margin.add_theme_constant_override("margin_bottom", 14)
	right_panel.add_child(right_margin)
	
	var right_vbox = VBoxContainer.new()
	right_vbox.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	right_vbox.add_theme_constant_override("separation", 8)
	right_margin.add_child(right_vbox)
	
	var hist_title = Label.new()
	hist_title.text = "MOVE HISTORY"
	hist_title.add_theme_font_size_override("font_size", 14)
	hist_title.add_theme_color_override("font_color", Color(0.9, 0.85, 0.7))
	hist_title.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	right_vbox.add_child(hist_title)
	
	right_vbox.add_child(HSeparator.new())
	
	history_label = RichTextLabel.new()
	history_label.bbcode_enabled = true
	history_label.size_flags_vertical = Control.SIZE_EXPAND_FILL
	history_label.add_theme_font_size_override("normal_font_size", 14)
	history_label.add_theme_color_override("default_color", Color(0.8, 0.82, 0.88))
	history_label.scroll_following = true
	right_vbox.add_child(history_label)
	

	var EngineBridgeScript = preload("res://scripts/engine_bridge.gd")
	engine = Node.new()
	engine.set_script(EngineBridgeScript)
	add_child(engine)
	engine.ai_move_found.connect(_on_ai_move_found)
	

	_start_game()


func _start_game():
	move_history = []
	is_game_over = false
	current_side = "w"
	_refresh_board()
	_fetch_legal_moves()
	
	if game_mode == "ai" and human_color != "w":

		_start_ai_turn()
	else:
		var side_name = "White" if current_side == "w" else "Black"
		_set_status("Your move (" + side_name + ")")

func _refresh_board():
	var state = engine.get_board_state(move_history)
	if state["rows"].size() == 8:
		board.update_board(state["rows"])
		current_side = state["side"]
	_update_captured_pieces()

func _fetch_legal_moves():
	all_legal_moves = engine.get_legal_moves(move_history)
	board.set_legal_moves(all_legal_moves)

func _update_captured_pieces():

	var start_counts = {"P": 8, "N": 2, "B": 2, "R": 2, "Q": 1, "K": 1,
						"p": 8, "n": 2, "b": 2, "r": 2, "q": 1, "k": 1}
	var current_counts = {}
	for key in start_counts:
		current_counts[key] = 0
	
	if board.board_data.size() == 8:
		for row in board.board_data:
			for piece in row:
				if piece in current_counts:
					current_counts[piece] += 1
	

	var white_lost = ""
	for p in ["Q", "R", "B", "N", "P"]:
		var lost = start_counts[p] - current_counts.get(p, 0)
		for i in range(max(0, lost)):
			white_lost += p + " "
	captured_white_label.text = "White lost: " + (white_lost if white_lost != "" else "—")
	

	var black_lost = ""
	for p in ["q", "r", "b", "n", "p"]:
		var lost = start_counts[p] - current_counts.get(p, 0)
		for i in range(max(0, lost)):
			black_lost += p + " "
	captured_black_label.text = "Black lost: " + (black_lost if black_lost != "" else "—")


func _on_move_attempt(move_str: String):
	if is_game_over or is_ai_thinking:
		return
	
	if move_str not in all_legal_moves:
		_set_status("Illegal move!")
		return
	

	move_history.append(move_str)
	_refresh_board()
	board.set_last_move(move_str)
	_add_move_to_history(move_str)
	_update_eval()
	

	var game_state = engine.check_game_over(move_history)
	if game_state != "no":
		_handle_game_over(game_state)
		return
	

	_update_check_highlight()
	
	if game_mode == "ai":
		_start_ai_turn()
	else:

		_fetch_legal_moves()
		var side_name = "White" if current_side == "w" else "Black"
		_set_status(side_name + "'s move")
		_show_hints()


func _start_ai_turn():
	is_ai_thinking = true
	board.interactive = false
	board.clear_selection()
	
	if opponent_type == "maia":
		_set_status("Maia is predicting player move...")
		thinking_anim.text = "⏳ Neural Network search..."
	else:
		_set_status("Minimax is thinking...")
		thinking_anim.text = "⏳ Depth " + str(ai_depth) + " IDS search..."
	
	engine.get_ai_move_async(move_history, ai_depth, opponent_type)

func _on_ai_move_found(result: Dictionary):
	is_ai_thinking = false
	board.interactive = true
	thinking_anim.text = ""
	
	if result["move"] == "":
		_set_status("AI has no moves!")
		return
	

	move_history.append(result["move"])
	_refresh_board()
	board.set_last_move(result["move"])
	_add_move_to_history(result["move"])
	_update_eval()
	

	var score_str = "%.2f" % (result["score"] / 100.0)
	var time_str = str(result["time"]) + "ms"
	
	if opponent_type == "maia":
		_set_status("Maia: " + result["move"] + " (human-like move, " + time_str + ")")
	else:
		_set_status("Minimax (IDS): " + result["move"] + " (eval " + score_str + ", " + str(result["nodes"]) + " nodes, " + time_str + ")")
	

	if result["state"] in ["checkmate", "stalemate", "draw"]:
		_handle_game_over(result["state"])
		return

	_update_check_highlight()
	

	_fetch_legal_moves()
	_show_hints()
	

	await get_tree().create_timer(1.2).timeout
	if not is_game_over and not is_ai_thinking:
		var side_name = "White" if current_side == "w" else "Black"
		_set_status("Your move (" + side_name + ")")


func _handle_game_over(state: String):
	is_game_over = true
	board.interactive = false
	board.clear_selection()
	
	match state:
		"checkmate":
			var loser = current_side
			var winner = "White" if loser == "b" else "Black"
			_set_status("♚ CHECKMATE! " + winner + " wins!")

			var king_code = "K" if loser == "w" else "k"
			board.set_check(board.find_king(king_code))
		"stalemate":
			_set_status("½ STALEMATE — Draw!")
		"draw":
			_set_status("½ DRAW — 50-move rule / insufficient material")

func _update_check_highlight():
	board.clear_check()

	var king_code = "K" if current_side == "w" else "k"
	var king_pos = board.find_king(king_code)

	var state = engine.check_game_over(move_history)
	if state == "no":

		pass

func _update_eval():
	var score = engine.get_eval(move_history)
	var score_f = score / 100.0
	var sign = "+" if score_f >= 0 else ""
	eval_label.text = "Eval: " + sign + ("%.2f" % score_f)

func _show_hints():
	if hint_label == null:
		return
	hint_label.clear()

	var hints = engine.get_hints(move_history, mini(ai_depth, 3))
	
	var mx_hints = []
	var maia_hints = []
	for h in hints:
		if h.get("engine", "") == "maia":
			maia_hints.append(h)
		else:
			mx_hints.append(h)
			
	if mx_hints.size() > 0:
		hint_label.append_text("[color=#a9c2f0][b]Minimax:[/b][/color]\n")
		for i in range(mx_hints.size()):
			var h = mx_hints[i]
			var score_f = h["score"] / 100.0
			var sign = "+" if score_f > 0 else ""
			hint_label.append_text("  " + str(i + 1) + ". " + h["move"] + " (" + sign + ("%.1f" % score_f) + ")\n")
			
	if maia_hints.size() > 0:
		hint_label.append_text("[color=#d4a9f0][b]Maia:[/b][/color]\n")
		for i in range(maia_hints.size()):
			var h = maia_hints[i]
			hint_label.append_text("  -> " + h["move"] + " (human-like)\n")

func _add_move_to_history(move_str: String):
	var move_num = (move_history.size() + 1) / 2
	if move_history.size() % 2 == 1:
		history_label.append_text("[b]" + str(move_num) + ".[/b] " + move_str + "  ")
	else:
		history_label.append_text(move_str + "\n")

func _set_status(text: String):
	if status_label:
		status_label.text = text


func _create_panel() -> PanelContainer:
	var panel = PanelContainer.new()
	var style = StyleBoxFlat.new()
	style.bg_color = Color(0.086, 0.129, 0.239, 0.9)  
	style.corner_radius_top_left = 8
	style.corner_radius_top_right = 8
	style.corner_radius_bottom_left = 8
	style.corner_radius_bottom_right = 8
	style.border_width_left = 1
	style.border_width_right = 1
	style.border_width_top = 1
	style.border_width_bottom = 1
	style.border_color = Color(0.2, 0.25, 0.4, 0.5)
	panel.add_theme_stylebox_override("panel", style)
	return panel

func _create_button(text: String) -> Button:
	var btn = Button.new()
	btn.text = text
	btn.custom_minimum_size = Vector2(0, 38)
	
	var normal_style = StyleBoxFlat.new()
	normal_style.bg_color = Color(0.059, 0.208, 0.376)  
	normal_style.corner_radius_top_left = 6
	normal_style.corner_radius_top_right = 6
	normal_style.corner_radius_bottom_left = 6
	normal_style.corner_radius_bottom_right = 6
	btn.add_theme_stylebox_override("normal", normal_style)
	
	var hover_style = StyleBoxFlat.new()
	hover_style.bg_color = Color(0.325, 0.204, 0.514)  
	hover_style.corner_radius_top_left = 6
	hover_style.corner_radius_top_right = 6
	hover_style.corner_radius_bottom_left = 6
	hover_style.corner_radius_bottom_right = 6
	btn.add_theme_stylebox_override("hover", hover_style)
	
	var pressed_style = StyleBoxFlat.new()
	pressed_style.bg_color = Color(0.914, 0.271, 0.376)  
	pressed_style.corner_radius_top_left = 6
	pressed_style.corner_radius_top_right = 6
	pressed_style.corner_radius_bottom_left = 6
	pressed_style.corner_radius_bottom_right = 6
	btn.add_theme_stylebox_override("pressed", pressed_style)
	
	btn.add_theme_color_override("font_color", Color(0.9, 0.9, 0.95))
	btn.add_theme_font_size_override("font_size", 14)
	
	return btn

func _on_back_pressed():

	get_tree().change_scene_to_file("res://scenes/main_menu.tscn")
	queue_free()
