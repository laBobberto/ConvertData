import pandas as pd
import altair as alt

file_map = {
    'clang_linux_benchmark_analysis.csv': 'Clang (Linux)',
    'g++_linux_benchmark_analysis.csv': 'G++ (Linux)',
    'mingw_win_benchmark_analysis.csv': 'MinGW (Win, per-call)',
    'msvc_win_benchmark_analysis.csv': 'MSVC (Win, per-call)',
    'time_mingw_win_benchmark_analysis.csv': 'MinGW (Win, batch)',
    'time_msvc_win_benchmark_analysis.csv': 'MSVC (Win, batch)'
}

data_frames = []

for filename, source_name in file_map.items():
    try:
        df = pd.read_csv(filename)
        df['Источник'] = source_name
        data_frames.append(df)
    except Exception as e:
        print(f"Не удалось загрузить файл {filename}: {e}")

if not data_frames:
    print("Данные не загружены. Невозможно построить графики.")
else:
    all_data = pd.concat(data_frames, ignore_index=True)

    version_sort_order = [
        'Original',
        'V1_EarlyReturn',
        'V2_BitOps_',
        'V4_Precalculation'
    ]

    base = alt.Chart(all_data).encode(
        x=alt.X('Источник', axis=None),

        column=alt.Column(
            'Версия',
            sort=version_sort_order,
            header=alt.Header(titleOrient="bottom", labelOrient="bottom", title='Версия бенчмарка')
        ),

        color=alt.Color('Источник', title='Компилятор (ОС)'),

        tooltip=['Версия', 'Источник', 'Среднее_нс', 'Медиана_нс', 'Пиковый_RSS_КБ', 'P95_нс', 'P99_нс']

    ).properties(
        width=120
    ).interactive()

    chart_avg = base.mark_bar().encode(
        y=alt.Y('Среднее_нс', title='Среднее время (нс)')
    ).properties(
        title='Среднее время выполнения (нс)'
    )

    chart_median = base.mark_bar().encode(
        y=alt.Y('Медиана_нс', title='Медианное время (нс)')
    ).properties(
        title='Медианное время выполнения (нс)'
    )

    chart_p95 = base.mark_bar().encode(
        y=alt.Y('P95_нс', title='P95 Время (нс)')
    ).properties(
        title='95-й перцентиль (нс)'
    )

    chart_p99 = base.mark_bar().encode(
        y=alt.Y('P99_нс', title='P99 Время (нс)')
    ).properties(
        title='99-й перцентиль (нс)'
    )

    chart_rss = base.mark_bar().encode(
        y=alt.Y('Пиковый_RSS_КБ', title='Пиковый RSS (КБ)')
    ).properties(
        title='Пиковое использование ОЗУ (КБ)'
    )

    final_chart = alt.vconcat(
        chart_avg,
        chart_median,
        chart_p95,
        chart_p99,
        chart_rss
    ).properties(
        title='Сводный анализ производительности C++ бенчмарков'
    ).resolve_scale(
        y='independent'
    )

    # final_chart_path = 'benchmark_summary_charts.json'
    # final_chart.save(final_chart_path)
    final_chart_path = 'benchmark_summary_charts.png'
    final_chart.save(final_chart_path)

    print(f"Все графики сохранены в {final_chart_path}")