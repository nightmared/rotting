if __FILE__ == $0
  if ARGV.size() < 2
    puts "USAGE: ruby thisfile.rb /path/to/file/of/subtitles.srt offset"
    puts "INFO: offset is a number of milliseconds such as 1000 or -2512"
    exit(-1)
  end
  File.open(ARGV[0]) do |file|
    file.each_line do |line|
      regexp = /(?<hours>\d{2}):(?<mins>\d{2}):(?<secs>\d{2})\,(?<millis>\d{3})/
      matched = line.scan regexp
      unless matched.empty?
        matched.each do |match|
          milliseconds = match[0].to_i() * 3600000 + match[1].to_i() * 60000 + match[2].to_i() * 1000 + match[3].to_i()
          new_time = Array.new
          milliseconds += ARGV[1].to_i
          [3600000, 60000, 1000, 1].each do |it|
            new_time.push(milliseconds/it)
            milliseconds -= (milliseconds/it)*it
          end
          line.sub!(regexp, "#{new_time[0]}:#{new_time[1]}:#{new_time[2]},#{new_time[3]}")
        end
      end
      print line
    end
  end
end
