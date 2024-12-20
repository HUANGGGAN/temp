import pandas as pd
import numpy as np

# 載入資料集
df = pd.read_csv('stage12_wash_dateFormatted_train.csv')

# 獲取所有以 "home_" 和 "away_" 開頭的欄位名稱
home_columns = [col for col in df.columns if col.startswith('home_')]
away_columns = [col for col in df.columns if col.startswith('away_')]

# 填補 "home_" 欄位的缺失值，依據 home_team_abbr
for home_col in home_columns:
    if home_col == 'home_team_abbr':
        continue  # 忽略 home_team_abbr 本身

    # 將非數值型的資料轉為 NaN
    df[home_col] = pd.to_numeric(df[home_col], errors='coerce')

    # 填補缺失值
    df[home_col] = df.groupby('home_team_abbr')[home_col].transform(
        lambda x: x.fillna(x.mean())
    )

# 填補 "away_" 欄位的缺失值，依據 away_team_abbr
for away_col in away_columns:
    if away_col == 'away_team_abbr':
        continue  # 忽略 away_team_abbr 本身

    # 將非數值型的資料轉為 NaN
    df[away_col] = pd.to_numeric(df[away_col], errors='coerce')

    # 填補缺失值
    df[away_col] = df.groupby('away_team_abbr')[away_col].transform(
        lambda x: x.fillna(x.mean())
    )

# 將結果輸出為新的 CSV
df.to_csv('stage12_wash_dateFormatted_filled_train.csv', index=False)

print("done")
