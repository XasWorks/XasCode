
require 'interpolate'

module Interpolate
	def self.mix_hashes(baseHash, maskHash, offset: 0, preSpacing: nil, postSpacing: nil, spacing: 0)
		preSpacing  ||= spacing;
		postSpacing ||= spacing;

		upperBound = maskHash.keys.max + postSpacing + offset;
		lowerBound = maskHash.keys.min - preSpacing  + offset;

		baseInterpolation = Interpolate::Points.new(baseHash);
		baseHashLowerBound = baseInterpolation.at(lowerBound);
		baseHashUpperBound = baseInterpolation.at(upperBound);

		baseHash.delete_if do |key, val| key.between? lowerBound, upperBound end

		baseHash[lowerBound] = baseHashLowerBound if preSpacing  > 0;
		baseHash[upperBound] = baseHashUpperBound if postSpacing > 0;

		maskHash.each do |k, v|
			baseHash[k+offset] = v;
		end

		return baseHash
	end

	def self.mix_looped(baseHash, maskHash, lowerBound: 0, upperBound: 1, offset: 0, preSpacing: nil, postSpacing: nil, spacing: 0)
		preSpacing  ||= spacing;
		postSpacing ||= spacing;

		upperMaskBound = maskHash.keys.max + postSpacing + offset;
		lowerMaskBound = maskHash.keys.min - preSpacing  + offset;

		raise ArgumentError, "maskHash is bigger than baseHash area!" if (upperMaskBound - lowerMaskBound) > (upperBound - lowerBound);

		mix_hashes(baseHash, maskHash, offset: offset, preSpacing: preSpacing, postSpacing: postSpacing);
		mix_hashes(baseHash, maskHash, offset: offset + (upperBound-lowerBound), preSpacing: preSpacing, postSpacing: postSpacing) if lowerMaskBound < lowerBound;
		mix_hashes(baseHash, maskHash, offset: offset - (upperBound-lowerBound), preSpacing: preSpacing, postSpacing: postSpacing) if upperMaskBound > upperBound;
	end
end
