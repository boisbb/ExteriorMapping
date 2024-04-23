import os
import pandas as pd
import numpy as np
import subprocess
import matplotlib.pyplot as plt
import json
import shutil

def call_command(command):
    process = subprocess.run(command.split(), capture_output=True)
    return process.stdout.decode("utf-8")

def create_samples_array(command):
    output = call_command(command)
    lines = output.split(sep='\n')
    lines = lines[lines.index("-----EVALUATION RESULTS-----") + 2:][:-1]
    indices = np.array([pair.split(sep=' ')[0] for pair in lines])
    colorValues = np.array([pair.split(sep=' ')[1] for pair in lines])

    return indices, colorValues

def evaluate_samples():
    indices, colorValues = create_samples_array('../build/ExteriorMapping --config by_step/config.json --eval samples c')
    indices, depthValues = create_samples_array('../build/ExteriorMapping --config by_step/config.json --eval samples d')
    indices, depthAngleValues = create_samples_array('../build/ExteriorMapping --config by_step/config.json --eval samples da')

    df = np.around(np.stack((indices, colorValues, depthValues, depthAngleValues), axis=1).astype(np.float64), 1)
    df = pd.DataFrame(df, columns=['samples', 'color', 'depth distance', 'depth angle'])

    fig, ax = plt.subplots()
    ax.set_xticks(np.arange(min(df['samples'].to_numpy()), max(df['samples'].to_numpy())+1, 32.0))
    ax.set_yticks(np.arange(10, max(df['depth distance'].to_numpy()) + 1, 10.0))
    
    df.set_index("samples", inplace=True)
    df.plot(ax=ax, grid=True, colormap='copper')
    ax.grid(color='lightgray', linestyle='--')
    ax.set_xlabel("number of samples per ray")
    ax.set_ylabel("mean render time [ms]")
    box = ax.get_position()
    ax.set_position([box.x0, box.y0, box.width * 0.8, box.height])
    ax.legend(loc='center left', bbox_to_anchor=(1, 0.5))
    plt.ylim(ymin=0)
    plt.xlim(xmin=0)

    plt.savefig('samples_plot.pdf')
    plt.show()

def acquire_cameras_data():
    configs = "../res/configs/"
    eval_json_dir = configs + 'eval'
    if not os.path.exists(eval_json_dir):
        os.makedirs(eval_json_dir)
    
    data = json.load(open("../res/configs/by_step/config.json"))

    df = []

    start = [1, 2]
    for i in range(13):
        if i % 2 == 0:
            start[0] += 1
        else:
            start[1] += 1

        data['viewData']['viewGrid']['gridSize']['x'] = start[0]
        data['viewData']['viewGrid']['gridSize']['y'] = start[1]

        config_file_name = "eval/" + str(start[0]) + str(start[1]) + ".json"
        f = open(configs + config_file_name, "w")
        json.dump(data, f, indent=4)
        f.close()

        output = call_command('../build/ExteriorMapping --config ' + config_file_name + ' --eval one d')
        lines = output.split(sep='\n')
        line = lines[lines.index("-----EVALUATION RESULTS-----") + 2:][:-1]
        line = line[0].split(sep=' ')
        line = [float(i) for i in line]

        df.append(line)

    shutil.rmtree(eval_json_dir)

    return df

def evaluate_cameras():
    df = acquire_cameras_data()

    df = np.around(np.array(df).astype(np.float64), 1)
    df = pd.DataFrame(df, columns=['views', 'time'])
    df.set_index("views", inplace=True)
    
    fig, ax = plt.subplots()
    ax.set_xticks([4, 6, 9, 12, 16, 20, 25, 30, 36, 42, 49, 56, 64])
    ax.set_yticks(np.arange(10, max(df['time'].to_numpy()) + 1, 30.0))
    
    df.plot(ax=ax, grid=True, colormap='copper')
    ax.grid(color='lightgray', linestyle='--')
    ax.set_xlabel("number of views")
    ax.set_ylabel("mean render time [ms]")
    ax.get_legend().remove()
    plt.ylim(ymin=0)
    plt.xlim(xmin=0)

    plt.savefig('cameras_plot.pdf')
    plt.show()

def evaluate_mse():
    return
    

def main():
    # evaluate_samples()
    evaluate_cameras()

if __name__ == "__main__":
    main()