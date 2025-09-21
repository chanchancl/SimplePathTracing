from PIL import Image
import os

# 设置输入和输出目录
input_dir = "output"
output_dir = "output_png"

# 确保输出目录存在
if not os.path.exists(output_dir):
    os.makedirs(output_dir)

# 遍历output目录中的所有文件
for filename in os.listdir(input_dir):
    if filename.endswith(".ppm"):
        # 构建输入和输出文件的完整路径
        input_path = os.path.join(input_dir, filename)
        output_filename = os.path.splitext(filename)[0] + ".png"
        output_path = os.path.join(output_dir, output_filename)

        # 打开并转换图片
        with Image.open(input_path) as img:
            # 转换为RGB（如果需要）
            img = img.convert("RGB")
            # 保存为JPG
            img.save(output_path, "PNG")
            print(f"Converted {filename} to {output_filename}")

print("Conversion completed!")
