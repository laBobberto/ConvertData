import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import os

if not os.path.exists('results.csv'):
    exit()

df = pd.read_csv('results.csv')

def get_speedup(row):
    baseline = df[(df['Version'] == 'Original') & (df['OptLevel'] == row['OptLevel'])]['Average_ns'].values[0]
    return baseline / row['Average_ns']

df['Speedup'] = df.apply(get_speedup, axis=1)

sns.set_theme(style="whitegrid", context="talk")
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(14, 14))

sns.barplot(data=df, x='Version', y='Average_ns', hue='OptLevel', ax=ax1)
ax1.set_title('')
ax1.set_ylabel('Time (ns)')

sns.barplot(data=df, x='Version', y='Speedup', hue='OptLevel', ax=ax2)
ax2.set_title('')
ax2.set_ylabel('Speedup (x)')
ax2.axhline(1, ls='--', color='red', alpha=0.5)

plt.tight_layout()
plt.savefig('i586_benchmark_charts_combined.png')
