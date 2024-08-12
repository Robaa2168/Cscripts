import enchant
from itertools import combinations

def insert_spaces(text):
    d = enchant.Dict("en_US")
    words = text.split()
    new_text = []

    for word in words:
        # If the word is already in the dictionary, append it as-is.
        if d.check(word):
            new_text.append(word)
        else:
            # Generate possible splits for the word
            splits = [(word[:i], word[i:]) for i in range(len(word) + 1)]
            valid_splits = [(a, b) for a, b in splits if d.check(a) and d.check(b)]
            
            # If no valid splits, try combinations of splits
            if not valid_splits:
                for i in range(1, len(word)):
                    for combo in combinations(range(1, len(word)), i):
                        parts = [word[i:j] for i, j in zip((0,) + combo, combo + (None,))]
                        if all(d.check(part) for part in parts):
                            new_text.extend(parts)
                            break
            else:
                # Choose the first valid split
                a, b = valid_splits[0]
                new_text.extend([a, b])
    
    return ' '.join(new_text)

# Example usage with your text:
original_text = "Accordingtobank 2014therecoveryprocessisdefinedasthesetofwellcoordinated..."
processed_text = insert_spaces(original_text)
print(processed_text)
