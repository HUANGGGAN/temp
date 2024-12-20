import pandas as pd
from sklearn.model_selection import train_test_split

# Read the CSV file into a DataFrame
df = pd.read_csv('stage2_train.csv')

# Split the DataFrame into 80% training data and 20% testing data
train_df, test_df = train_test_split(df, test_size=0.2, random_state=42)

# Save the train and test DataFrames into new CSV files
train_df.to_csv('stage2_train.csv', index=False)
test_df.to_csv('stage2_validitation.csv', index=False)

print("Dataset split completed! Train data: 'train_data.csv', Test data: 'test_data.csv'")
