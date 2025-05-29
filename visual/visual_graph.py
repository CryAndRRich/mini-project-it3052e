import matplotlib.pyplot as plt

def plot_cp_mip_exact_sol(cp, mip, title):
    plt.figure(figsize=(10, 6))
    plt.plot(cp, color="blue", label=f"CP", linestyle="-")
    plt.plot(mip, color="brown", label=f"MIP", linestyle="-")

    plt.xlabel("Test ID")
    if title == "cost":
        plt.ylabel(f"Total cost")
        plt.title(f"Total cost (lower is better)")
    if title == "time":
        plt.ylabel(f"Runtime (ms)")
        plt.title(f"Runtime (lower is better)")
    plt.legend()
    plt.xticks(range(0, len(cp)))
    plt.tight_layout()
    plt.show()

def plot_all_solutions(data, title):
    colors = ["blue", "brown", "red", "yellow", "green"]
    plt.figure(figsize=(10, 6))

    idx = 0
    for key, vals in data.items():
        plt.plot(vals, label=key, color=colors[idx], linestyle='-')
        idx += 1

    plt.xlabel("Test ID")
    if title == "cost":
        plt.ylabel(f"Total cost")
        plt.title(f"Total cost (lower is better)")
    if title == "time":
        plt.ylabel(f"Runtime (ms)")
        plt.title(f"Runtime (lower is better)")
    plt.legend()
    plt.xticks(range(0, len(data["CP"]), 5))
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    # cp_f = [5, 3, 3, 3, 4, 4, 6, 4, 5, 8, 9, 8, 10, 7, 8, 5]
    # cp_t = [11, 12, 14, 20, 30, 26, 33, 84, 44, 97, 93, 100, 157, 170, 12576, 2162]
    # mip_f = [5, 3, 3, 3, 4, 4, 6, 4, 5, 8, 9, 8, None, None, None, None]
    # mip_t = [76, 82, 294, 261, 103, 153, 407, 537, 1086, 1262, 25633, 22657, None, None, None, None]
    # plot_cp_mip_exact_sol(cp_f, mip_f, "cost")
    # plot_cp_mip_exact_sol(cp_t, mip_t, "time")

    # cp_f = [5, 3, 3, 3, 4, 4, 6, 4, 5, 8, 9, 8, 10, 7, 8, 5, 7, 8, 14, 9, 14, 9, 9, 9, 18, 184, None, None, None, None, None, None, None, None, None, None]
    # cp_t = [11, 12, 15, 20, 30, 26, 34, 84, 44, 97, 93, 100, 157, 170, 12576, 2162, 300042, 300049, 1653, 300043, 300311, 300179, 300180, 300191, 300355, 301983, None, None, None, None, None, None, None, None, None, None]
    # mip_f = [5, 3, 3, 3, 4, 4, 6, 4, 5, 8, 9, 8, 10, 7, 8, 5, 9, 15, 15, 9, 15, 13, 18, 23, 17, None, None, None, None, None, None, None, None, None, None]
    # mip_t = [76, 82, 294, 261, 103, 152, 407, 537, 1086, 1262, 25633, 22657, 303677, 301622, 304671, 318972, 320001, 322745, 315434, 314809, 311737, 326745, 336635, 335098, 342410, None, None, None, None, None, None, None, None, None, None]
    # f_min_SA = [5, 3, 3, 3, 4, 4, 6, 4, 5, 8, 9, 8, 10, 7, 8, 5, 7, 8, 14, 10, 14, 9, 9, 9, 9, 18, 31, 47, 62, 69, 90, 89, 158, 125, 184, 120, 174, 178]
    # t_avg_SA = [51, 29, 30, 27, 37, 35, 44, 34, 36, 50, 45, 42, 38, 36, 37, 44, 42, 42, 61, 50, 57, 47, 44, 50, 45, 63, 89, 120, 153, 181, 220, 242, 360, 344, 472, 360, 850, 1888]
    # f_min_HS = [5, 3, 4, 3, 4, 4, 6, 4, 6, 8, 10, 8, 10, 8, 9, 10, 10, 10, 15, 11, 16, 11, 12, 12, 12, 21, 34, 51, 68, 73, 92, 94, 161, 130, 186, 125, 180, 180]
    # t_avg_HS = [3, 4, 4, 5, 4, 4, 5, 4, 5, 5, 5, 5, 7, 6, 7, 14, 15, 16, 16, 16, 16, 16, 16, 18, 17, 26, 50, 74, 99, 121, 146, 168, 194, 219, 240, 246, 557, 770]
    # f_min_BS = [5, 3, 3, 3, 4, 4, 6, 4, 5, 8, 9, 8, 10, 7, 8, 8, 8, 9, 14, 11, 14, 10, 10, 11, 11, 20, 33, 50, 64, 73, 91, 92, 160, 129, 184, 122, None, None]
    # t_avg_BS = [7, 42, 51, 72, 68, 80, 94, 119, 104, 64, 82, 90, 118, 180, 161, 1316, 1591, 1698, 1311, 942, 730, 1627, 2272, 1270, 2910, 2135, 6046, 12639, 22114, 42933, 53999, 112749, 104058, 148392, 179579, 368464, None, None]

    # plot_all_solutions(
    #     {
    #         "CP": cp_t, 
    #         "MIP" : mip_t, 
    #         "SA" : t_avg_SA, 
    #         "HS": t_avg_HS, 
    #         "BS": t_avg_BS
    #     }, 
    #    "time"
    # )
    pass
