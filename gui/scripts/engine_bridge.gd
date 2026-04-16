
extends Node

signal ai_move_found(result: Dictionary)

var engine_path: String = ""
var _thread: Thread = null

func _ready():
	if OS.has_feature("editor"):
		var project_dir = ProjectSettings.globalize_path("res://")
		engine_path = project_dir.path_join("chess.exe")
		if not FileAccess.file_exists(engine_path):

			engine_path = project_dir.get_base_dir().path_join("chess.exe")
	else:
		var exe_dir = OS.get_executable_path().get_base_dir()
		engine_path = exe_dir.path_join("chess.exe")
		if not FileAccess.file_exists(engine_path):
			push_error("chess.exe not found alongside exported executable in: " + exe_dir)


func _send_commands(commands: PackedStringArray) -> String:
	var temp_dir = OS.get_user_data_dir()
	var cmd_file = temp_dir.path_join("chess_cmd.txt")
	

	var f = FileAccess.open(cmd_file, FileAccess.WRITE)
	if not f:
		push_error("Cannot write command file: " + cmd_file)
		return ""
	for cmd in commands:
		f.store_line(cmd)
	f.store_line("quit")
	f.close()
	

	var cmd_file_win = cmd_file.replace("/", "\\")
	var engine_win = engine_path.replace("/", "\\")
	var pipe_cmd = 'type "' + cmd_file_win + '" | "' + engine_win + '" --protocol'
	
	var output: Array = []
	OS.execute("cmd.exe", ["/c", pipe_cmd], output, true, false)
	
	if output.size() > 0:
		return output[0]
	return ""


func _build_replay_commands(move_history: Array) -> PackedStringArray:
	var commands: PackedStringArray = PackedStringArray()
	commands.append("new")
	for m in move_history:
		commands.append("move " + m)
	return commands


func _parse_responses(raw: String) -> PackedStringArray:
	var lines = raw.split("\n")
	var result: PackedStringArray = PackedStringArray()
	for line in lines:
		var trimmed = line.strip_edges()
		if trimmed != "" and trimmed != "ready" and trimmed != "bye":
			result.append(trimmed)
	return result




func get_board_state(move_history: Array) -> Dictionary:
	var commands = _build_replay_commands(move_history)
	commands.append("board")
	
	var output = _send_commands(commands)
	var responses = _parse_responses(output)
	
	var rows: Array = []
	var side = "w"
	for r in responses:
		if r.begins_with("row"):
			var parts = r.split(" ")
			var row_data: PackedStringArray = PackedStringArray()
			for i in range(1, parts.size()):
				row_data.append(parts[i])
			rows.append(row_data)
		elif r.begins_with("endboard"):
			var parts = r.split(" ")
			if parts.size() > 1:
				side = parts[1]
	
	return {"rows": rows, "side": side}


func get_legal_moves(move_history: Array) -> PackedStringArray:
	var commands = _build_replay_commands(move_history)
	commands.append("legal")
	
	var output = _send_commands(commands)
	var responses = _parse_responses(output)
	
	for r in responses:
		if r.begins_with("moves"):
			var parts = r.split(" ")
			var moves: PackedStringArray = PackedStringArray()
			for i in range(1, parts.size()):
				if parts[i].strip_edges() != "":
					moves.append(parts[i].strip_edges())
			return moves
	
	return PackedStringArray()


func check_game_over(move_history: Array) -> String:
	var commands = _build_replay_commands(move_history)
	commands.append("gameover")
	
	var output = _send_commands(commands)
	var responses = _parse_responses(output)
	
	for r in responses:
		if r.begins_with("gameover"):
			var parts = r.split(" ")
			if parts.size() > 1:
				return parts[1].strip_edges()
	return "no"


func get_eval(move_history: Array) -> int:
	var commands = _build_replay_commands(move_history)
	commands.append("eval")
	
	var output = _send_commands(commands)
	var responses = _parse_responses(output)
	
	for r in responses:
		if r.begins_with("eval"):
			var parts = r.split(" ")
			if parts.size() > 1:
				return int(parts[1])
	return 0


func get_ai_move(move_history: Array, depth: int, opponent_type: String = "minimax") -> Dictionary:
	var commands = _build_replay_commands(move_history)
	commands.append("go " + opponent_type + " " + str(depth))
	
	var output = _send_commands(commands)
	var responses = _parse_responses(output)
	
	for r in responses:
		if r.begins_with("bestmove"):
			return _parse_bestmove(r)
	
	return {"move": "", "score": 0, "nodes": 0, "time": 0, "state": ""}


func get_ai_move_async(move_history: Array, depth: int, opponent_type: String = "minimax"):
	if _thread != null and _thread.is_started():
		_thread.wait_to_finish()
	_thread = Thread.new()
	_thread.start(_ai_thread_func.bind(move_history.duplicate(), depth, opponent_type))

func _ai_thread_func(move_history: Array, depth: int, opponent_type: String):
	var result = get_ai_move(move_history, depth, opponent_type)
	call_deferred("_on_ai_done", result)

func _on_ai_done(result: Dictionary):
	if _thread != null and _thread.is_started():
		_thread.wait_to_finish()
	ai_move_found.emit(result)


func get_hints(move_history: Array, depth: int) -> Array:
	var commands = _build_replay_commands(move_history)
	commands.append("hints " + str(depth))
	
	var output = _send_commands(commands)
	var responses = _parse_responses(output)
	
	var hints: Array = []
	for r in responses:
		if r.begins_with("hint "):
			var parts = r.split(" ")
			if parts.size() >= 4:
				hints.append({
					"engine": parts[1],
					"move": parts[2],
					"score": int(parts[3])
				})
	return hints


func _parse_bestmove(line: String) -> Dictionary:
	var result = {"move": "", "score": 0, "nodes": 0, "time": 0, "state": ""}
	var parts = line.split(" ")
	
	var i = 0
	while i < parts.size():
		match parts[i]:
			"bestmove":
				if i + 1 < parts.size():
					result["move"] = parts[i + 1]
			"score":
				if i + 1 < parts.size():
					result["score"] = int(parts[i + 1])
			"nodes":
				if i + 1 < parts.size():
					result["nodes"] = int(parts[i + 1])
			"time":
				if i + 1 < parts.size():
					result["time"] = int(parts[i + 1])
			"checkmate", "stalemate", "draw", "check":
				result["state"] = parts[i]
		i += 1
	
	return result

func _exit_tree():
	if _thread != null and _thread.is_started():
		_thread.wait_to_finish()
