import os
import PIL.Image
import pandas as pd
import numpy as np
import subprocess
import matplotlib.pyplot as plt
import json
import shutil
import PIL
import tqdm

ROOT = "../"
EXECUTABLE = '../build/ExteriorMapping'
GRAPHS_PATH = 'graphs/'

EVAL_FRAMES = 20

CAMERAS_SAMPLES = 32

MSE_SAMPLES = 179
MSE_REGENERATE = False
MSE_DELETE = False

def call_command(command):
    print("Running command:\n", command)

    process = subprocess.run(command.split(), capture_output=True)
    return process.stdout.decode("utf-8")

def create_samples_array(command):
    output = call_command(command)
    lines = output.split(sep='\n')
    lines = lines[lines.index("-----EVALUATION RESULTS-----") + 2:][:-1]
    indices = np.array([pair.split(sep=' ')[0] for pair in lines])
    values = np.array([pair.split(sep=' ')[1] for pair in lines])

    return indices, values

def create_samples_array_gt(command):
    output = call_command(command)
    lines = output.split(sep='\n')
    lines = lines[lines.index("-----EVALUATION RESULTS-----") + 2:][:-1]
    value = lines[0]

    return float(value)

def evaluate_samples():
    indices, colorValues = create_samples_array(EXECUTABLE + ' --config by_step/config.json --eval samples c ' + str(EVAL_FRAMES))
    indices, depthValues = create_samples_array(EXECUTABLE + ' --config by_step/config.json --eval samples d ' + str(EVAL_FRAMES))
    indices, depthAngleValues = create_samples_array(EXECUTABLE + ' --config by_step/config.json --eval samples da ' + str(EVAL_FRAMES))
    # gt_value = create_samples_array_gt(EXECUTABLE + ' --config by_step/config.json --eval gt ' + str(RAY_SAMPLES_FRAMES))
    # gt_values = np.ones(colorValues.shape[0]) * gt_value

    df = np.around(np.stack((indices, colorValues, depthValues, depthAngleValues), axis=1).astype(np.float64), 1)
    df = pd.DataFrame(df, columns=['samples', 'color heuristic', 'depth euler heuristic', 'depth angle heuristic'])

    fig, ax = plt.subplots()
    ax.set_xticks(np.arange(0, 256, 32.0))
    ax.set_yticks(np.arange(10, max(df['depth euler heuristic'].to_numpy()) + 1, 10.0))
    
    df.set_index("samples", inplace=True)
    df.plot(ax=ax, grid=True, colormap='copper')
    ax.grid(color='lightgray', linestyle='--')
    ax.set_xlabel("number of samples per ray")
    ax.set_ylabel("mean render time [ms]")
    # box = ax.get_position()
    # ax.set_position([box.x0, box.y0, box.width * 0.8, box.height])
    # ax.legend(loc='center left', bbox_to_anchor=(1, 0.5))
    plt.ylim(ymin=0)
    plt.xlim(xmin=0, xmax=256)
    # plt.yscale('symlog')

    plt.savefig(os.path.join(GRAPHS_PATH, 'samples_plot.pdf'))
    plt.show()

def generate_eval_dir():
    configs = os.path.join(ROOT, "res/configs/")
    eval_json_dir = os.path.join(configs, 'eval')

    if not os.path.exists(eval_json_dir):
        os.makedirs(eval_json_dir)

    data = json.load(open(os.path.join(ROOT, "res/configs/by_step/config.json")))

    start = [1, 2]
    eval_files = []

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

        eval_files.append(config_file_name)

    return eval_files

def acquire_cameras_data(heuristic, eval_files):
    df = []
    df_indices = []

    start = [1, 2]
    for i in range(13):
        if i % 2 == 0:
            start[0] += 1
        else:
            start[1] += 1

        print ("Evaluating file:", eval_files[i], str(i + 1) + "/13")

        output = call_command(EXECUTABLE + ' --config ' + eval_files[i] + ' --eval one ' + heuristic + ' ' + str(CAMERAS_SAMPLES) + ' ' + str(EVAL_FRAMES))
        lines = output.split(sep='\n')
        line = lines[lines.index("-----EVALUATION RESULTS-----") + 2:][:-1]
        line = line[0].split(sep=' ')

        df_indices.append(line[0])
        df.append(line[1])
    return np.array(df_indices), np.array(df)

def evaluate_cameras():
    configs = os.path.join(ROOT, "res/configs/")
    eval_json_dir = os.path.join(configs, 'eval')

    eval_files = generate_eval_dir()

    _, df_c = acquire_cameras_data('c', eval_files)
    indices, df_d = acquire_cameras_data('d', eval_files)
    _, df_da = acquire_cameras_data('da', eval_files)

    shutil.rmtree(eval_json_dir)

    df = np.around(np.stack((indices, df_c, df_d, df_da), axis=1).astype(np.float64), 1)
    # df = np.around(np.array(df).astype(np.float64), 1)
    df = pd.DataFrame(df, columns=['views', 'color heuristic', 'depth euler heuristic', 'depth angle heuristic'])
    df.set_index("views", inplace=True)
    
    fig, ax = plt.subplots()
    ax.set_xticks([4, 6, 9, 12, 16, 20, 25, 30, 36, 42, 49, 56, 64])
    ax.set_yticks(np.arange(10, max(df['depth euler heuristic'].to_numpy()) + 1, 30.0))
    
    df.plot(ax=ax, grid=True, colormap='copper')
    ax.grid(color='lightgray', linestyle='--')
    ax.set_xlabel("number of views")
    ax.set_ylabel("mean render time [ms]")
    plt.ylim(ymin=0)
    plt.xlim(xmin=0)

    plt.savefig(os.path.join(GRAPHS_PATH, 'cameras_plot.pdf'))
    plt.show()

def generate_images():
    call_command(EXECUTABLE + ' --config by_step/config.json --eval mse c ' + str(MSE_SAMPLES))
    call_command(EXECUTABLE + ' --config by_step/config.json --eval mse d ' + str(MSE_SAMPLES))
    call_command(EXECUTABLE + ' --config by_step/config.json --eval mse da ' + str(MSE_SAMPLES))
    call_command(EXECUTABLE + ' --config by_step/config.json --eval mse gt')
    return

def images_mse(gt_folder, novel_folder):
    gt_files = [f for f in os.listdir(gt_folder) if os.path.isfile(os.path.join(gt_folder, f))]
    novel_files = [f for f in os.listdir(novel_folder) if os.path.isfile(os.path.join(novel_folder, f))]

    gt_files.sort()
    novel_files.sort()

    overall_diff = None
    overall_mse = 0

    for i in tqdm.tqdm(range(len(gt_files))):
        gt = gt_files[i]
        novel = novel_files[i]
        gt_img = PIL.Image.open(os.path.join(gt_folder, gt))
        novel_img = PIL.Image.open(os.path.join(novel_folder, novel))

        gt_img = np.asarray(gt_img).astype('float')
        novel_img = np.asarray(novel_img).astype('float')

        gt_img /= 255
        novel_img /= 255

        diff = (gt_img - novel_img) ** 2

        if overall_diff is None:
            overall_diff = diff
        else:
            overall_diff += diff

        err = np.sum(diff)
        mse = err / float(gt_img.shape[0] * gt_img.shape[1])

        overall_mse += mse
    
    overall_diff = np.sum(overall_diff, axis=2)
    overall_mse /= len(gt_files)

    overall_diff /= overall_diff.max()

    return overall_mse, overall_diff


def evaluate_mse():
    print("----MSE Evaluation----")

    if MSE_REGENERATE:
        generate_images()

    screenshots_eval = os.path.join(ROOT, 'screenshots/eval/')
    gt_folder = os.path.join(screenshots_eval, 'gt')

    folder_dict = {
        "color": os.path.join(screenshots_eval, "novel_c"),
        "dist": os.path.join(screenshots_eval, 'novel_d'),
        "dist_angle": os.path.join(screenshots_eval, 'novel_da')
    }
    
    for k, folder in folder_dict.items():
        print("Evaluating for ", k)

        mse, diff = images_mse(gt_folder, folder)

        diff /= 150

        print(diff)

        fig, ax = plt.subplots(1, 1)
        ax.imshow(diff)

        print("MSE for", k, "heuristic is: ", mse, "\n")
        plt.savefig(os.path.join(GRAPHS_PATH, 'heatmap_' + k + '.pdf'))

    if MSE_DELETE:
        shutil.rmtree(screenshots_eval)
    

def main():
    if not os.path.exists(GRAPHS_PATH):
        os.makedirs(GRAPHS_PATH)

    evaluate_samples()
    evaluate_cameras()
    evaluate_mse()

if __name__ == "__main__":
    main()
