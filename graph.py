import csv
import os
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

file_map = {
    'clang_linux_benchmark_analysis.csv': 'Clang (Linux)',
    'g++_linux_benchmark_analysis.csv': 'G++ (Linux)',
    'mingw_win_benchmark_analysis.csv': 'MinGW (Win, per-call)',
    'msvc_win_benchmark_analysis.csv': 'MSVC (Win, per-call)',
    'time_mingw_win_benchmark_analysis.csv': 'MinGW (Win, batch)',
    'time_msvc_win_benchmark_analysis.csv': 'MSVC (Win, batch)'
}

metrics = ['Average_ns', 'Median_ns', 'P95_ns', 'P99_ns']
all_results = []

for filename, source_name in file_map.items():
    if not os.path.exists(filename):
        continue
        
    try:
        with open(filename, mode='r', encoding='utf-8') as f:
            reader = csv.DictReader(f)
            source_data = list(reader)
            
            baseline = None
            for row in source_data:
                version = row['Version'].strip('_')
                if version == 'Original':
                    baseline = float(row['Average_ns'])
                    break
            
            if baseline is None:
                continue
                
            for row in source_data:
                version = row['Version'].strip('_')
                avg_ns = float(row['Average_ns'])
                median_ns = float(row['Median_ns'])
                p95_ns = float(row['P95_ns'])
                p99_ns = float(row['P99_ns'])
                speedup = baseline / avg_ns if avg_ns > 0 else 0
                
                all_results.append({
                    'Source': source_name,
                    'Version': version,
                    'Average_ns': avg_ns,
                    'Median_ns': median_ns,
                    'P95_ns': p95_ns,
                    'P99_ns': p99_ns,
                    'Speedup': speedup
                })
    except Exception as e:
        print(f"Error processing {filename}: {e}")

if not all_results:
    print("No results to process.")
    exit()

df = pd.DataFrame(all_results)
df.to_csv('performance_comparison_detailed.csv', index=False)

# Main Summary Plot (Average & Speedup)
sns.set_theme(style="whitegrid", context="talk")
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(14, 12))

sns.barplot(data=df, x='Source', y='Average_ns', hue='Version', ax=ax1)
ax1.set_title('Average Execution Time (ns) - Lower is Better')
ax1.set_ylabel('Time (ns)')
ax1.set_xlabel('')
ax1.tick_params(axis='x', rotation=15)
ax1.legend(title='Version', bbox_to_anchor=(1.05, 1), loc='upper left')

sns.barplot(data=df, x='Source', y='Speedup', hue='Version', ax=ax2)
ax2.set_title('Speedup vs Original (Average) - Higher is Better')
ax2.set_ylabel('Speedup (x)')
ax2.set_xlabel('Compiler / Platform')
ax2.tick_params(axis='x', rotation=15)
ax2.axhline(1, ls='--', color='red', alpha=0.5)
ax2.legend(title='Version', bbox_to_anchor=(1.05, 1), loc='upper left')

plt.tight_layout()
plt.savefig('benchmark_summary_charts.png')

# Detailed Plot (Tail Latencies)
fig_det, (ax3, ax4) = plt.subplots(2, 1, figsize=(14, 12))

df_melted_tail = df.melt(id_vars=['Source', 'Version'], value_vars=['P95_ns', 'P99_ns'], var_name='Metric', value_name='Time_ns')
sns.barplot(data=df_melted_tail, x='Source', y='Time_ns', hue='Version', ax=ax3)
ax3.set_title('Tail Latency (P95 & P99) - Lower is Better')
ax3.set_ylabel('Time (ns)')
ax3.set_xlabel('')
ax3.tick_params(axis='x', rotation=15)
ax3.legend(title='Version', bbox_to_anchor=(1.05, 1), loc='upper left')

# Speedup Heatmap (Version vs Source)
pivot_speedup = df.pivot(index='Version', columns='Source', values='Speedup')
sns.heatmap(pivot_speedup, annot=True, fmt=".2f", cmap="YlGnBu", ax=ax4)
ax4.set_title('Speedup Relative to Original (Heatmap)')

plt.tight_layout()
plt.savefig('benchmark_summary_charts_detailed.png')

print("\n=== SUMMARY ANALYSIS ===")
for source in df['Source'].unique():
    source_df = df[df['Source'] == source]
    best_row = source_df.loc[source_df['Speedup'].idxmax()]
    print(f"{source:<25}: Best is {best_row['Version']} ({best_row['Speedup']:.2f}x speedup)")

print("\nCharts updated:")
print("- benchmark_summary_charts.png: Average Time and Speedup")
print("- benchmark_summary_charts_detailed.png: Tail Latencies and Speedup Heatmap")
print("- performance_comparison_detailed.csv: Raw data table")
