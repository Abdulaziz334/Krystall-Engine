import json

class AIOS:
    def __init__(self):
        self.config = {}
        self.token_bank = {}
        self.world_state = {}

    def init_system(self):
        with open("config.json", "r") as f:
            self.config = json.load(f)
        print("⚙️ AIOS sozlamalari yuklandi:", self.config)

    def get_user_data(self, user_id):
        return self.world_state.get(user_id, {"location": "spawn", "inventory": []})

    def update_user_data(self, user_id, data):
        self.world_state[user_id] = data

    def manage_tokens(self, user_id, amount):
        self.token_bank[user_id] = self.token_bank.get(user_id, 0) + amount
        return self.token_bank[user_id]